#include "example_joiner.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"

// VW headers
#include "example.h"
#include "parse_example_json.h"
#include "parser.h"
#include "v_array.h"

int default_reward_calculation(const joined_event &event) {
  std::cout << "this is the default reward logic" << std::endl;
  return err::success;
}

ExampleJoiner::ExampleJoiner(const std::string &initial_command_line,
                             RewardCalcType rc)
    : _initial_command_line(initial_command_line), reward_calculation(rc),
      _example_pool(VW::initialize(initial_command_line)) {}

ExampleJoiner::~ExampleJoiner() = default;

example *ExampleJoiner::get_or_create_example() {
  return _example_pool.get_or_create_example();
}

example &ExampleJoiner::get_or_create_example_f(void *example_joiner) {
  return *(((ExampleJoiner *)example_joiner)->get_or_create_example());
}

int ExampleJoiner::process_event(const v2::JoinedEvent &joined_event) {
  auto event = flatbuffers::GetRoot<v2::Event>(joined_event.event()->data());
  auto metadata = event->meta();
  std::cout << "id:" << metadata->id()->c_str()
            << " type:" << v2::EnumNamePayloadType(metadata->payload_type())
            << " payload-size:" << event->payload()->size()
            << " encoding:" << v2::EnumNameEventEncoding(metadata->encoding())
            << std::endl;

  if (metadata->encoding() == v2::EventEncoding_Zstd) {
    std::cout << "Decompression coming soon" << std::endl;
  }

  if (metadata->payload_type() == v2::PayloadType_Outcome) {
    process_outcome(*event, *metadata);
  } else {
    process_interaction(*event, *metadata);
  }

  return err::success;
}

int ExampleJoiner::process_interaction(const v2::Event &event,
                                       const v2::Metadata &metadata) {

  metadata_info meta = {"client_time_utc",
                        metadata.app_id() ? metadata.app_id()->str() : "",
                        metadata.payload_type(), metadata.pass_probability(),
                        metadata.encoding()};

  if (metadata.payload_type() == v2::PayloadType_CB) {
    auto cb = v2::GetCbEvent(event.payload()->data());
    std::cout << std::endl
              << "cb: actions:"
              << (cb->action_ids() == nullptr ? 0 : cb->action_ids()->size())
              << " model:" << cb->model_id()->c_str()
              << " lm:" << v2::EnumNameLearningModeType(cb->learning_mode())
              << " deferred:" << cb->deferred_action() << std::endl
              << "context:" << cb->context()->data() << std::endl;

    for (size_t j = 0; j < cb->action_ids()->size(); j++) {
      std::cout << "action:" << cb->action_ids()->Get(j)
                << " prob:" << cb->probabilities()->Get(j) << std::endl
                << std::endl;
    }

    auto examples = v_init<example *>();
    examples.push_back(_example_pool.get_or_create_example());

    std::vector<char> line_vec(cb->context()->data(),
                               cb->context()->data() + cb->context()->size() +
                                   1);

    DecisionServiceInteraction data;
    data.eventId = metadata.id()->str();
    data.actions = {cb->action_ids()->data(),
                    cb->action_ids()->data() + cb->action_ids()->size()};
    data.probabilities = {cb->probabilities()->data(),
                          cb->probabilities()->data() +
                              cb->probabilities()->size()};
    data.probabilityOfDrop = metadata.pass_probability();
    data.skipLearn = cb->deferred_action();

    VW::read_line_json<false>(*_example_pool.get_vw(), examples, &line_vec[0],
                              get_or_create_example_f, this);

    joined_event info = {"joined_event_timestamp",
                         examples,
                         std::move(meta),
                         std::move(data),
                         {}};
    _unjoined_examples.emplace(metadata.id()->str(), std::move(info));
  }
  return err::success;
}

int ExampleJoiner::process_outcome(const v2::Event &event,
                                   const v2::Metadata &metadata) {

  outcome_event o_event;
  o_event.metadata = {"client_time_utc",
                      metadata.app_id() ? metadata.app_id()->str() : "",
                      metadata.payload_type(), metadata.pass_probability(),
                      metadata.encoding()};

  auto outcome = v2::GetOutcomeEvent(event.payload()->data());

  int index = -1;

  std::cout << "outcome: value:";
  if (outcome->value_type() == v2::OutcomeValue_literal) {
    o_event.s_value = outcome->value_as_literal()->c_str();
    std::cout << outcome->value_as_literal()->c_str();
  } else if (outcome->value_type() == v2::OutcomeValue_numeric) {
    o_event.value = outcome->value_as_numeric()->value();
    std::cout << outcome->value_as_numeric()->value();
  }

  std::cout << " index:";
  if (outcome->index_type() == v2::IndexValue_literal) {
    std::cout << outcome->index_as_literal()->c_str();
    o_event.s_index = outcome->index_as_literal()->c_str();
    index = std::stoi(outcome->index_as_literal()->c_str());
  } else if (outcome->index_type() == v2::IndexValue_numeric) {
    std::cout << outcome->index_as_numeric()->index();
    o_event.s_index = outcome->index_as_numeric()->index();
    index = outcome->index_as_numeric()->index();
  }

  std::cout << " action-taken:" << outcome->action_taken() << std::endl;

  if (_unjoined_examples.find(metadata.id()->str()) !=
      _unjoined_examples.end()) {
    auto &joined_event = _unjoined_examples[metadata.id()->str()];
    joined_event.outcome_events.push_back(o_event);
  }

  return err::success;
}

int ExampleJoiner::train_on_joined() {
  for (auto &example : _unjoined_examples) {
    auto &joined_event = _unjoined_examples[example.first];

    // call logic that creates the reward
    reward_calculation(joined_event);

    VW::setup_examples(*_example_pool.get_vw(), joined_event.examples);
    multi_ex examples2(joined_event.examples.begin(),
                       joined_event.examples.end());

    _example_pool.get_vw()->learn(examples2);
    // clean up examples and push examples back into pool for re-use
    _example_pool.get_vw()->finish_example(examples2);
    for (auto &&ex : joined_event.examples) {
      _example_pool.return_example(ex);
    }

    // cleanup
    joined_event.examples.delete_v();
  }
  _unjoined_examples.clear();

  return err::success;
}