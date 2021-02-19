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

    // table Metadata {

    //     client_time_utc:TimeStamp;
    //     app_id:string;
    //     payload_type:PayloadType;
    // encoding: EventEncoding;
    // }

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
    // VW::read_line_decision_service_json<false>(
    //     *_vw, examples, &line_vec[0], line_vec.size(), false /*copy line*/,
    //     get_or_create_example_f, this, &data);

    event_info info = {"timestamp", examples, std::move(data)};
    _unjoined_examples.emplace(metadata.id()->str(), std::move(info));
  }
  return err::success;
}

int ExampleJoiner::process_outcome(const flatbuffers::Vector<uint8_t> *payload,
                                   const v2::Metadata &metadata) {

  auto outcome = v2::GetOutcomeEvent(payload->data());

  int index = -1;

  std::cout << "outcome: value:";
  if (outcome->value_type() == v2::OutcomeValue_literal) {
    std::cout << outcome->value_as_literal()->c_str();
  } else if (outcome->value_type() == v2::OutcomeValue_numeric) {
    std::cout << outcome->value_as_numeric()->value();
  }

  std::cout << " index:";
  if (outcome->index_type() == v2::IndexValue_literal) {
    std::cout << outcome->index_as_literal()->c_str();
    index = std::stoi(outcome->index_as_literal()->c_str());
  } else if (outcome->index_type() == v2::IndexValue_numeric) {
    std::cout << outcome->index_as_numeric()->index();
    index = outcome->index_as_numeric()->index();
  }

  std::cout << " action-taken:" << outcome->action_taken() << std::endl;

  std::string id(metadata.id()->str());

  if (_unjoined_examples.find(id) != _unjoined_examples.end()) {
    auto &event_info = _unjoined_examples[id];
    if (index == -1) {
      index = event_info.interaction_data.actions[0];
    }
    // dummy join
    // dummy reward
    event_info.examples[index]->l.cb.costs.push_back(
        {1.0f, event_info.interaction_data.actions[index - 1],
         event_info.interaction_data.probabilities[index - 1]});

    _ready_events.push_back(id);
  }

  return err::success;
}

int ExampleJoiner::train_on_joined() {
  for (auto &id : _ready_events) {
    auto &event_info = _unjoined_examples[id];
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

    _unjoined_examples.erase(id);
  }
  _ready_events.clear();
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

//   reinforcement_learning::safe_vw _vw;
//   // from dictionary id to example object
//   // right now holding one dedup dictionary at a time, could be exented to a
//   map
//   // of maps holding more than one dedup dictionaries at a time
//   std::unordered_map<std::string, example *> dedup_examples;
//   // from event id to example object(s)
//   std::unordered_map<std::string, v_array<example *>> unjoined_examples;
//   // from event id to example metadata
//   std::unordered_map<std::string, event_info> unjoined_examples_info;
