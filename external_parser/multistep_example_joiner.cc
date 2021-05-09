#include "multistep_example_joiner.h"

#include "generated/v2/DedupInfo_generated.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "io/logger.h"

#include <limits.h>
#include <time.h>

// VW headers
#include "example.h"
#include "io/logger.h"
#include "parse_example_json.h"
#include "parser.h"
#include "v_array.h"


multistep_example_joiner::multistep_example_joiner(vw *vw)
    : _vw(vw), _reward_calculation(&RewardFunctions::earliest) {}

multistep_example_joiner::~multistep_example_joiner() {
  // cleanup examples
  for (auto *ex : _example_pool) {
    VW::dealloc_examples(ex, 1);
  }
}

int multistep_example_joiner::process_event(const v2::JoinedEvent &joined_event) {
  auto event = flatbuffers::GetRoot<v2::Event>(joined_event.event()->data());
  const v2::Metadata& meta = *event->meta();
  std::string id = meta.id()->str();
  switch (meta.payload_type()) {
    case v2::PayloadType_MultiStep:
    {
      auto interaction = flatbuffers::GetRoot<v2::MultiStepEvent>(event->payload()->data());
      _interactions[interaction->event_id()->str()].push_back({meta, *interaction});
      break;
    }
    case v2::PayloadType_Outcome:
    {
      auto outcome = flatbuffers::GetRoot<v2::OutcomeEvent>(event->payload()->data());
      const char* id =  outcome->index_type() == v2::IndexValue_literal ? outcome->index_as_literal()->c_str() : nullptr;
      if (id == nullptr) {
        _episodic_outcomes.push_back({meta, *outcome});
      } else {
        _outcomes[std::string(id)].push_back({meta, *outcome});
      }
      break;    
    }
    default:
      break;
  }
  return 0;
}

void multistep_example_joiner::set_default_reward(float default_reward) {
  _default_reward = default_reward;
}

void multistep_example_joiner::set_learning_mode_config(const v2::LearningModeType& learning_mode) {
  _learning_mode_config = learning_mode;
}

void multistep_example_joiner::set_problem_type_config(const v2::ProblemType& problem_type) {
  _problem_type_config = problem_type;
}

void multistep_example_joiner::set_reward_function(const v2::RewardFunctionType type) {
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

void multistep_example_joiner::populate_order() {
  for (const auto it: _interactions) {
    _order.push(it.first);
  }
}

outcome_event multistep_example_joiner::process_outcome(const multistep_example_joiner::Parsed<v2::OutcomeEvent> &event_meta) {
  const auto& metadata = event_meta.meta;
  const auto& event = event_meta.event;
  outcome_event o_event;
  o_event.metadata = {"client_time_utc",
                      metadata.app_id() ? metadata.app_id()->str() : "",
                      metadata.payload_type(), metadata.pass_probability(),
                      metadata.encoding()};

  if (event.value_type() == v2::OutcomeValue_literal) {
    o_event.s_value = event.value_as_literal()->c_str();
  } else if (event.value_type() == v2::OutcomeValue_numeric) {
    o_event.value = event.value_as_numeric()->value();
  }

  return o_event;
}

joined_event multistep_example_joiner::process_interaction(
    const multistep_example_joiner::Parsed<v2::MultiStepEvent> &event_meta,
    v_array<example *> &examples) {
  const auto& metadata = event_meta.meta;
  const auto& event = event_meta.event;
  metadata_info meta = {"client_time_utc",
                        metadata.app_id() ? metadata.app_id()->str() : "",
                        metadata.payload_type(),
                        metadata.pass_probability(),
                        metadata.encoding(),
                        v2::LearningModeType::LearningModeType_Online};

  DecisionServiceInteraction data;
  data.eventId = event.event_id()->str();
  data.actions = {event.action_ids()->data(),
                  event.action_ids()->data() + event.action_ids()->size()};
  data.probabilities = {event.probabilities()->data(),
                        event.probabilities()->data() +
                        event.probabilities()->size()};
  data.probabilityOfDrop = 1.f - metadata.pass_probability();
  data.skipLearn = false;//cb->deferred_action();

  std::string line_vec(reinterpret_cast<char const *>(event.context()->data()),
                        event.context()->size());

  if (_vw->audit || _vw->hash_inv) {
    VW::template read_line_json<true>(
        *_vw, examples, const_cast<char *>(line_vec.c_str()),
        reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), _vw);
  } else {
    VW::template read_line_json<false>(
        *_vw, examples, const_cast<char *>(line_vec.c_str()),
        reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), _vw);
  }
  return {"joiner_timestamp", std::move(meta), std::move(data)};
}

void try_set_label(const joined_event &je, float reward,
                                   v_array<example *> &examples) {
  if (je.interaction_data.actions.empty()) {
    VW::io::logger::log_error("missing actions for event [{}]",
                              je.interaction_data.eventId);
    return;
  }

  if (je.interaction_data.probabilities.empty()) {
    VW::io::logger::log_error("missing probabilities for event [{}]",
                              je.interaction_data.eventId);
    return;
  }

  if (std::any_of(je.interaction_data.probabilities.begin(),
                  je.interaction_data.probabilities.end(),
                  [](float p) { return std::isnan(p); })) {
    VW::io::logger::log_error(
        "distribution for event [{}] contains invalid probabilities",
        je.interaction_data.eventId);
  }

  int index = je.interaction_data.actions[0];
  auto action = je.interaction_data.actions[0];
  auto cost = -1.f * reward;
  auto probability = je.interaction_data.probabilities[0] *
                     (1.f - je.interaction_data.probabilityOfDrop);
  auto weight = 1.f - je.interaction_data.probabilityOfDrop;

  examples[index]->l.cb.costs.push_back({cost, action, probability});
  examples[index]->l.cb.weight = weight;
}

int multistep_example_joiner::process_joined(v_array<example *> &examples) {
  if (!_sorted) {
    populate_order();
  }
  const auto& id = _order.front();

  const auto& interactions = _interactions[id];
  if (id.size() != 1) {
    return -1;
  }
  const auto& interaction = interactions[0];
  auto joined = process_interaction(interaction, examples);

  const auto outcomes = _outcomes[id];
  for (const auto& o: outcomes) {
    joined.outcome_events.push_back(process_outcome(o));
  }
  for (const auto& o: _episodic_outcomes) {
    joined.outcome_events.push_back(process_outcome(o));
  }
  const auto reward = _reward_calculation(joined);
  try_set_label(joined, reward, examples);
  _order.pop();
  return 0;
}

bool multistep_example_joiner::processing_batch() {
  return _sorted && !_order.empty();
}

void multistep_example_joiner::on_new_batch() {
  _interactions.clear();
  _outcomes.clear();
  _episodic_outcomes.clear();
  _sorted = false;
}
