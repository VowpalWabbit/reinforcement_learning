#include "example_joiner.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"

#include <limits.h>

// VW headers
#include "example.h"
#include "parse_example_json.h"
#include "parser.h"
#include "v_array.h"

namespace RewardFunctions {
float average(const joined_event &event) {
  float sum = 0.f;
  for (const auto &o : event.outcome_events) {
    sum += o.value;
  }

  return sum / event.outcome_events.size();
}

float sum(const joined_event &event) {
  float sum = 0.f;
  for (const auto &o : event.outcome_events) {
    sum += o.value;
  }

  return sum;
}

float min(const joined_event &event) {
  float min_reward = std::numeric_limits<float>::max();
  for (const auto &o : event.outcome_events) {
    if (o.value < min_reward) {
      min_reward = o.value;
    }
  }
  return min_reward;
}

float max(const joined_event &event) {
  float max_reward = std::numeric_limits<float>::min();
  for (const auto &o : event.outcome_events) {
    if (o.value > max_reward) {
      max_reward = o.value;
    }
  }
  return max_reward;
}

float median(const joined_event &event) {
  std::vector<float> values;
  for (const auto &o : event.outcome_events) {
    values.push_back(o.value);
  }

  int outcome_events_size = values.size();

  sort(values.begin(), values.end());
  if (outcome_events_size % 2 == 0) {
    return (values[outcome_events_size / 2 - 1] +
            values[outcome_events_size / 2]) /
           2;
  } else {
    return values[outcome_events_size / 2];
  }
}

float earliest(const joined_event &event) { return 0.0; }
} // namespace RewardFunctions

example_joiner::example_joiner(vw *vw)
    : _vw(vw), _reward_calculation(&RewardFunctions::earliest) {}

int example_joiner::process_event(const v2::JoinedEvent &joined_event) {
  auto event = flatbuffers::GetRoot<v2::Event>(joined_event.event()->data());
  std::string id = event->meta()->id()->str();
  if (_batch_grouped_events.find(id) != _batch_grouped_events.end()) {
    _batch_grouped_events[id].push_back(event);
  } else {
    _batch_grouped_events.insert({id, {event}});
    _batch_event_order.emplace(id);
  }
  return 0;
}

void example_joiner::set_default_reward(float default_reward) {
  _default_reward = default_reward;
}

void example_joiner::set_reward_function(const v2::RewardFunctionType type) {
  using namespace RewardFunctions;

  switch (type) {
  case v2::RewardFunctionType_Earliest:
    _reward_calculation = &earliest;
    break;
  case v2::RewardFunctionType_Average:
    _reward_calculation = &average;
    break;

  case v2::RewardFunctionType_Sum:
    _reward_calculation = &sum;
    break;

  case v2::RewardFunctionType_Min:
    _reward_calculation = &min;
    break;

  case v2::RewardFunctionType_Max:
    _reward_calculation = &max;
    break;

  case v2::RewardFunctionType_Median:
    _reward_calculation = &median;
    break;

  default:
    break;
  }
}

int example_joiner::process_interaction(const v2::Event &event,
                                        const v2::Metadata &metadata,
                                        v_array<example *> &examples) {

  if (metadata.payload_type() == v2::PayloadType_CB) {
    auto cb = v2::GetCbEvent(event.payload()->data());
    std::cout << std::endl
              << "cb: actions:"
              << (cb->action_ids() == nullptr ? 0 : cb->action_ids()->size())
              << " model:" << cb->model_id()->c_str()
              << " lm:" << v2::EnumNameLearningModeType(cb->learning_mode())
              << " deferred:" << cb->deferred_action() << std::endl
              << "context:" << cb->context()->data() << std::endl;
    v2::LearningModeType learning_mode = cb->learning_mode();

    metadata_info meta = {"client_time_utc",
                          metadata.app_id() ? metadata.app_id()->str() : "",
                          metadata.payload_type(),
                          metadata.pass_probability(),
                          metadata.encoding(),
                          learning_mode};

    for (size_t j = 0; j < cb->action_ids()->size(); j++) {
      std::cout << "action:" << cb->action_ids()->Get(j)
                << " prob:" << cb->probabilities()->Get(j) << std::endl
                << std::endl;
    }

    DecisionServiceInteraction data;
    data.eventId = metadata.id()->str();
    data.actions = {cb->action_ids()->data(),
                    cb->action_ids()->data() + cb->action_ids()->size()};
    data.probabilities = {cb->probabilities()->data(),
                          cb->probabilities()->data() +
                              cb->probabilities()->size()};
    data.probabilityOfDrop = metadata.pass_probability();
    data.skipLearn = cb->deferred_action();

    std::string line_vec(reinterpret_cast<char const *>(cb->context()->data()),
                         cb->context()->size());

    VW::read_line_json<false>(
        *_vw, examples, const_cast<char *>(line_vec.c_str()),
        reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), _vw);

    _batch_grouped_examples.emplace(std::make_pair<std::string, joined_event>(
        metadata.id()->str(),
        {"joiner_timestamp", std::move(meta), std::move(data)}));
  }
  return 0;
}

int example_joiner::process_outcome(const v2::Event &event,
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

  if (_batch_grouped_examples.find(metadata.id()->str()) !=
      _batch_grouped_examples.end()) {
    auto &joined_event = _batch_grouped_examples[metadata.id()->str()];
    joined_event.outcome_events.push_back(o_event);
  }

  return 0;
}

int example_joiner::process_joined(v_array<example *> &examples) {
  if (_batch_event_order.empty()) {
    return 0;
  }

  auto &id = _batch_event_order.front();

  bool multiline = true;
  for (auto *event : _batch_grouped_events[id]) {
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
      if (metadata->payload_type() == v2::PayloadType_CA) {
        multiline = false;
      }
      process_interaction(*event, *metadata, examples);
    }
  }
  // call logic that creates the reward
  float reward = _default_reward;
  auto &je = _batch_grouped_examples[id];
  if (je.outcome_events.size() > 0) {
    if (je.interaction_metadata.payload_type == v2::PayloadType_CB &&
        je.interaction_metadata.learning_mode ==
            v2::LearningModeType_Apprentice) {
      if (je.interaction_data.actions[0] == 1) {
        reward = _reward_calculation(je);
      }
    } else {
      reward = _reward_calculation(je);
    }
  }

  int index = je.interaction_data.actions[0];
  examples[index]->l.cb.costs.push_back(
      {1.0f, je.interaction_data.actions[index - 1],
       je.interaction_data.probabilities[index - 1]});

  std::cout << "reward value: " << reward << std::endl;

  if (multiline) {
    // add an empty example to signal end-of-multiline
    examples.push_back(&VW::get_unused_example(_vw));
  }

  _batch_grouped_events.erase(id);
  _batch_event_order.pop();
  _batch_grouped_examples.erase(id);

  return 0;
}

bool example_joiner::processing_batch() { return !_batch_event_order.empty(); }