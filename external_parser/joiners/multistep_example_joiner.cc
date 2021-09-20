#include "joiners/multistep_example_joiner.h"
#include "parse_example_external.h"

#include "generated/v2/DedupInfo_generated.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "io/logger.h"
#include "event_processors/typed_events.h"
#include "utils.h"

#include <limits.h>
#include <time.h>
#include <map>
#include <stack>
#include <queue>
#include <tuple>
#include <cstdio>

// VW headers
#include "example.h"
#include "io/logger.h"
#include "parse_example_json.h"
#include "parser.h"
#include "v_array.h"


multistep_example_joiner::multistep_example_joiner(vw *vw)
    : _vw(vw)
    , _reward_calculation(&reward::earliest)
    , _multistep_reward_calculation(&multistep_reward_suffix_mean) {}

multistep_example_joiner::~multistep_example_joiner() {
  // cleanup examples
  for (auto *ex : _example_pool) {
    VW::dealloc_examples(ex, 1);
  }
}

bool multistep_example_joiner::process_event(const v2::JoinedEvent &joined_event) {
  auto event = flatbuffers::GetRoot<v2::Event>(joined_event.event()->data());
  const v2::Metadata& meta = *event->meta();
  auto enqueued_time_utc = get_enqueued_time(joined_event.timestamp(),
                                               meta.client_time_utc(),
                                               _loop_info.use_client_time);
  switch (meta.payload_type()) {
    case v2::PayloadType_MultiStep:
    {
      auto interaction = flatbuffers::GetRoot<v2::MultiStepEvent>(event->payload()->data());
      _interactions[interaction->event_id()->str()].push_back({enqueued_time_utc, meta, *interaction});
      break;
    }
    case v2::PayloadType_Outcome:
    {
      auto outcome = flatbuffers::GetRoot<v2::OutcomeEvent>(event->payload()->data());
      const char* id =  outcome->index_type() == v2::IndexValue_literal ? outcome->index_as_literal()->c_str() : nullptr;
      if (id == nullptr) {
        _episodic_outcomes.push_back(process_outcome(enqueued_time_utc, meta, *outcome));
      } else {
        _outcomes[std::string(id)].push_back(process_outcome(enqueued_time_utc, meta, *outcome));
      }
      break;
    }
    default:
      break;
  }
  return true;
}

void multistep_example_joiner::set_default_reward(float default_reward, bool sticky) {
  _loop_info.default_reward.set(default_reward, sticky);
}

void multistep_example_joiner::set_learning_mode_config(
    v2::LearningModeType learning_mode, bool sticky) {
  _loop_info.learning_mode_config.set(learning_mode, sticky);
}

void multistep_example_joiner::set_problem_type_config(v2::ProblemType problem_type, bool sticky) {
  _loop_info.problem_type_config.set(problem_type, sticky);
}

void multistep_example_joiner::set_use_client_time(bool use_client_time, bool sticky) {
  _loop_info.use_client_time.set(use_client_time, sticky);
}

bool multistep_example_joiner::joiner_ready() {
  return _loop_info.is_configured() && _reward_calculation.is_valid();
}

void multistep_example_joiner::set_reward_function(const v2::RewardFunctionType type, bool sticky) {

  reward::RewardFunctionType reward_calculation = nullptr;
  switch (type) {
  case v2::RewardFunctionType_Earliest:
    reward_calculation = &reward::earliest;
    break;
  case v2::RewardFunctionType_Average:
    reward_calculation = &reward::average;
    break;

  case v2::RewardFunctionType_Sum:
    reward_calculation = &reward::sum;
    break;

  case v2::RewardFunctionType_Min:
    reward_calculation = &reward::min;
    break;

  case v2::RewardFunctionType_Max:
    reward_calculation = &reward::max;
    break;

  case v2::RewardFunctionType_Median:
    reward_calculation = &reward::median;
    break;

  default:
    break;
  }

  if(reward_calculation) {
    _reward_calculation.set(reward_calculation, sticky);
  }
}

void multistep_example_joiner::set_multistep_reward_function(const multistep_reward_funtion_type type, bool sticky) {

  MultistepRewardFunctionType result = nullptr;
  switch (type) {
  case multistep_reward_funtion_type::SuffixMean:
    result = &multistep_reward_suffix_mean;
    break;
  case multistep_reward_funtion_type::SuffixSum:
    result = &multistep_reward_suffix_sum;
    break;
  case multistep_reward_funtion_type::Identity:
    result = &multistep_reward_identity;
    break;
  default:
    break;
  }

  if(result) {
    _multistep_reward_calculation.set(result, sticky);
  }
}

/*
take forest of tuples <id, secondary> as input.
Edges are defined using optional previous_id parameter.
get return list of ids ordered topologically with respect to <previous_id, id> edges and 
according to comp_t comparison for vertices that are not connected
*/
template<typename id_t, typename secondary_t, typename comp_t = std::greater<std::tuple<secondary_t, id_t>>>
class topo_sorter {
public:
  using elem_t = std::tuple<secondary_t, id_t>;
  using layer_t = std::priority_queue<elem_t, std::vector<elem_t>, comp_t>;

private:
  std::map<id_t, layer_t> next;
  layer_t roots;

public:
  void push(const id_t& id, const secondary_t& secondary) {
    roots.push(std::make_tuple(secondary, id));
  }

  void push(const id_t& id, const secondary_t& secondary, const id_t& previous_id) {
    next[previous_id].push(std::make_tuple(secondary, id));
  }

  void get(std::deque<id_t>& result) {
    std::stack<layer_t*> states;
    states.emplace(&roots);
    while (!states.empty()) {
      auto& top = *(states.top());
      if (!top.empty()) {
        const auto& cur = top.top();
        const auto& cur_id = std::get<1>(cur); 
        result.push_back(cur_id);
        states.push(&next[cur_id]);
        top.pop();
      }
      else {
        states.pop();
      }
    }
  } 
};

bool multistep_example_joiner::populate_order() {
  topo_sorter<std::string, TimePoint> sorter;
  for (const auto& it: _interactions) {
    const auto& parsed = it.second[0];
    if (parsed.event.previous_id() == nullptr) {
      sorter.push(it.first, parsed.timestamp);
    } else {
      sorter.push(it.first, parsed.timestamp, parsed.event.previous_id()->str());
    }
  }
  sorter.get(_order);
  _sorted = true;
  return true;
}

reward::outcome_event multistep_example_joiner::process_outcome(
  const TimePoint timestamp, const v2::Metadata &metadata, const v2::OutcomeEvent& event) {
  reward::outcome_event o_event;
  o_event.metadata = {metadata.app_id() ? metadata.app_id()->str() : "",
                      metadata.payload_type(),
                      metadata.pass_probability(),
                      metadata.encoding(),
                      metadata.id()->str()};

  if (event.value_type() == v2::OutcomeValue_literal) {
    o_event.s_value = event.value_as_literal()->c_str();
  } else if (event.value_type() == v2::OutcomeValue_numeric) {
    o_event.value = event.value_as_numeric()->value();
  }
  o_event.action_taken = event.action_taken();
  return o_event;
}

joined_event::joined_event multistep_example_joiner::process_interaction(
    const multistep_example_joiner::Parsed<v2::MultiStepEvent> &event_meta,
    v_array<example *> &examples) {
  const auto& metadata = event_meta.meta;
  const auto& event = event_meta.event;
  metadata::event_metadata_info meta = {
                        metadata.app_id() ? metadata.app_id()->str() : "",
                        metadata.payload_type(),
                        metadata.pass_probability(),
                        metadata.encoding(),
                        metadata.id()->str(),
                        v2::LearningModeType::LearningModeType_Online};

  auto cb_data = VW::make_unique<joined_event::cb_joined_event>();

  cb_data->interaction_data.eventId = event.event_id()->str();
  cb_data->interaction_data.actions = {event.action_ids()->data(),
                  event.action_ids()->data() + event.action_ids()->size()};
  cb_data->interaction_data.probabilities = {event.probabilities()->data(),
                        event.probabilities()->data() +
                        event.probabilities()->size()};
  cb_data->interaction_data.probabilityOfDrop = 1.f - metadata.pass_probability();
  cb_data->interaction_data.skipLearn = event.deferred_action();

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

  return joined_event::joined_event(
      TimePoint(event_meta.timestamp), std::move(meta), std::string(line_vec),
      std::string(event.model_id() ? event.model_id()->c_str() : "N/A"),
      std::move(cb_data));
}

bool multistep_example_joiner::process_joined(v_array<example *> &examples) {
  _current_je_is_skip_learn = false;

  if (!_sorted) {
    if (!populate_order()) {
      return false;
    }
  }
  const auto& id = _order.front();
  const float reward = _rewards.front();
  const auto& interactions = _interactions[id];
  if (interactions.size() != 1) {
    return false;
  }
  const auto& interaction = interactions[0];
  auto joined = process_interaction(interaction, examples);

  const auto outcomes = _outcomes[id];
  for (const auto& o: outcomes) {
    joined.outcome_events.push_back(o);
  }
  for (const auto& o: _episodic_outcomes) {
    joined.outcome_events.push_back(o);
  }

  bool clear_examples = false;
  auto guard = VW::scope_exit([&] {
    _order.pop_front();
    _rewards.pop_front();
    if (clear_examples) {
      VW::return_multiple_example(*_vw, examples);
      examples.push_back(&VW::get_unused_example(_vw));
    }
  });

  if (!joined.is_joined_event_learnable()) {
    _current_je_is_skip_learn = true;
    clear_examples = true;
    return false;
  }

  dynamic_cast<joined_event::cb_joined_event*>(joined.typed_data.get())->reward = reward;
 // joined.calc_reward(_loop_info.default_reward, _reward_calculation.value());
  joined.fill_in_label(examples);

  // add an empty example to signal end-of-multiline
  examples.push_back(&VW::get_unused_example(_vw));
  _vw->example_parser->lbl_parser.default_label(&examples.back()->l);
  examples.back()->is_newline = true;

  return true;
}

bool multistep_example_joiner::processing_batch() {
  return _sorted && !_order.empty();
}

void multistep_example_joiner::on_new_batch() {
  _interactions.clear();
  _outcomes.clear();
  _episodic_outcomes.clear();
  _rewards.clear();
  _sorted = false;
}

void multistep_example_joiner::populate_episodic_rewards() {
  for (const std::string& id: _order) {
    std::vector<reward::outcome_event> outcomes = _episodic_outcomes;
    const auto outcomes_per_step = _outcomes[id];
    outcomes.insert(outcomes.end(), std::make_move_iterator(outcomes_per_step.begin()), 
                    std::make_move_iterator(outcomes_per_step.end()));
    _rewards.push_back(_reward_calculation.value()(outcomes, _loop_info.default_reward));
  }
  _multistep_reward_calculation.value()(_rewards);
}

void multistep_example_joiner::on_batch_read() {
  populate_order();
  _sorted = true;
  populate_episodic_rewards();
}

metrics::joiner_metrics multistep_example_joiner::get_metrics()
{
  return _joiner_metrics;
}

bool multistep_example_joiner::current_event_is_skip_learn() {
  return _current_je_is_skip_learn;
}

void multistep_example_joiner::apply_cli_overrides(vw *all, const input_options &parsed_options) {
  if(all->options->was_supplied("multistep_reward")) {
    multistep_reward_funtion_type multistep_reward_func;

    if(!VW::external::str_to_enum(parsed_options.ext_opts->multistep_reward, multistep_reward_functions, multistep_reward_funtion_type::Identity, multistep_reward_func)) {
      throw std::runtime_error("Invalid argument to --multistep_reward " + parsed_options.ext_opts->reward_function);
    }
    set_multistep_reward_function(multistep_reward_func, true);
  }
}