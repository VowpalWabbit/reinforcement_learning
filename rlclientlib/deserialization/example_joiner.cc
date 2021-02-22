#include "example_joiner.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"

// VW headers
#include "example.h"
#include "parse_example_json.h"
#include "parser.h"
#include "v_array.h"

ExampleJoiner::ExampleJoiner(const std::string &initial_command_line)
    : _initial_command_line(initial_command_line),
      _vw(VW::initialize(initial_command_line)) {}

ExampleJoiner::~ExampleJoiner() {
  // cleanup examples
  for (auto &&ex : _example_pool) {
    VW::dealloc_example(_vw->example_parser->lbl_parser.delete_label, *ex);
    ::free_it(ex);
  }

  VW::finish(*_vw);
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
    process_outcome(event->payload(), *metadata);
  } else {
    process_interaction(event->payload(), *metadata);
  }

  return err::success;
}

int ExampleJoiner::process_interaction(
    const flatbuffers::Vector<uint8_t> *payload, const v2::Metadata &metadata) {

  metadata_info meta = {"client_time_utc",
                        metadata.app_id() ? metadata.app_id()->str() : "",
                        metadata.payload_type(), metadata.pass_probability(),
                        metadata.encoding()};

  if (metadata.payload_type() == v2::PayloadType_CB) {
    auto cb = v2::GetCbEvent(payload->data());
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
    examples.push_back(get_or_create_example());

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

    // DecisionServiceInteraction data will be used in creating the example
    VW::read_line_json<false>(*_vw, examples, &line_vec[0],
                              get_or_create_example_f, this);

    event_info info = {"joined_event_timestamp",
                       examples,
                       std::move(meta),
                       std::move(data),
                       {}};
    _unjoined_examples.emplace(metadata.id()->str(), std::move(info));
  }
  return err::success;
}

int ExampleJoiner::process_outcome(const flatbuffers::Vector<uint8_t> *payload,
                                   const v2::Metadata &metadata) {

  outcome_event o_event;
  o_event.metadata = {"client_time_utc",
                      metadata.app_id() ? metadata.app_id()->str() : "",
                      metadata.payload_type(), metadata.pass_probability(),
                      metadata.encoding()};

  auto outcome = v2::GetOutcomeEvent(payload->data());

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

  std::string id(metadata.id()->str());

  if (_unjoined_examples.find(id) != _unjoined_examples.end()) {
    auto &event_info = _unjoined_examples[id];
    event_info.outcome_events.push_back(o_event);
  }

  return err::success;
}

int ExampleJoiner::train_on_joined() {
  for (auto &example : _unjoined_examples) {
    auto &event_info = _unjoined_examples[example.first];

    // dummy join
    // dummy reward
    // join logic to be inserted somewhere here
    int index = event_info.interaction_data.actions[0];
    event_info.examples[index]->l.cb.costs.push_back(
        {1.0f, event_info.interaction_data.actions[index - 1],
         event_info.interaction_data.probabilities[index - 1]});

    VW::setup_examples(*_vw, event_info.examples);
    multi_ex examples2(event_info.examples.begin(), event_info.examples.end());

    _vw->learn(examples2);
    // clean up examples and push examples back into pool for re-use
    VW::finish_example(*_vw, examples2);
    for (auto &&ex : event_info.examples) {
      _example_pool.emplace_back(ex);
    }

    // cleanup
    event_info.examples.delete_v();
  }
  _unjoined_examples.clear();

  return err::success;
}

example *ExampleJoiner::get_or_create_example() {
  // alloc new element if we don't have any left
  if (_example_pool.size() == 0) {
    auto ex = VW::alloc_examples(0, 1);
    _vw->example_parser->lbl_parser.default_label(&ex->l);

    return ex;
  }

  // get last element
  example *ex = _example_pool.back();
  _example_pool.pop_back();

  VW::empty_example(*_vw, *ex);
  _vw->example_parser->lbl_parser.default_label(&ex->l);

  return ex;
}

example &ExampleJoiner::get_or_create_example_f(void *vw) {
  return *(((ExampleJoiner *)vw)->get_or_create_example());
}
