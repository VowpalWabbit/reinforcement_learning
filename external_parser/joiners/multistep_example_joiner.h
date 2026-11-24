#pragma once

#include "event_processors/joined_event.h"
#include "event_processors/loop.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "generated/v2/MultiStepEvent_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "joiners/i_joiner.h"
#include "metrics/metrics.h"
#include "parse_example_external.h"
#include "vw/core/example.h"
#include "vw/core/v_array.h"

#include <deque>
#include <fstream>
#include <list>
#include <queue>
#include <unordered_map>
// VW headers
// vw.h has to come before json_utils.h
// clang-format off
#include "vw/core/vw.h"
// clang-format on

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

enum multistep_reward_funtion_type
{
  Identity = 0,
  SuffixSum = 1,
  SuffixMean = 2
};

std::map<const char*, multistep_reward_funtion_type> const multistep_reward_functions = {{
    {"identity", multistep_reward_funtion_type::Identity},
    {"suffix_sum", multistep_reward_funtion_type::SuffixSum},
    {"suffix_mean", multistep_reward_funtion_type::SuffixMean},
}};

using MultistepRewardFunctionType = void (*)(std::deque<float>&);

inline void multistep_reward_identity(std::deque<float>&) {}

inline void multistep_reward_suffix_sum(std::deque<float>& rewards)
{
  for (size_t i = 1; i < rewards.size(); ++i) { rewards[rewards.size() - 1 - i] += rewards[rewards.size() - i]; }
}

inline void multistep_reward_suffix_mean(std::deque<float>& rewards)
{
  multistep_reward_suffix_sum(rewards);
  for (size_t i = 0; i < rewards.size(); ++i) { rewards[i] /= rewards.size() - i; }
}

class multistep_example_joiner : public i_joiner
{
public:
  multistep_example_joiner(VW::workspace* vw);  // TODO rule of 5
  multistep_example_joiner(VW::workspace* vw, bool binary_to_json, const std::string& outfile_name);

  ~multistep_example_joiner() override;

  void set_reward_function(const v2::RewardFunctionType type, bool sticky) override;
  void set_default_reward(float default_reward, bool sticky) override;
  void set_learning_mode_config(v2::LearningModeType learning_mode, bool sticky) override;
  void set_problem_type_config(v2::ProblemType problem_type, bool sticky) override;
  void set_use_client_time(bool use_client_time, bool sticky = false) override;
  void apply_cli_overrides(VW::workspace* all, const VW::external::parser_options& parsed_options) override;
  bool joiner_ready() override;

  bool current_event_is_skip_learn() override;

  bool process_event(const v2::JoinedEvent& joined_event) override;
  bool process_joined(VW::multi_ex& examples) override;
  bool processing_batch() override;

  void on_new_batch() override;
  void on_batch_read() override;
  metrics::joiner_metrics get_metrics() override;

private:
  template <typename event_t>
  struct Parsed
  {
    const TimePoint timestamp;
    const v2::Metadata& meta;
    const event_t& event;
  };
  void set_multistep_reward_function(const multistep_reward_funtion_type type, bool sticky);

private:
  bool populate_order();
  reward::outcome_event process_outcome(
      const TimePoint& timestamp, const v2::Metadata& metadata, const v2::OutcomeEvent& event);
  joined_event::joined_event process_interaction(
      const Parsed<v2::MultiStepEvent>& event_meta, VW::multi_ex& examples, float reward);
  void populate_episodic_rewards();

private:
  std::vector<VW::example*> _example_pool;

  VW::workspace* _vw;
  flatbuffers::DetachedBuffer _detached_buffer;

  loop::sticky_value<reward::RewardFunctionType> _reward_calculation;
  loop::sticky_value<MultistepRewardFunctionType> _multistep_reward_calculation;
  loop::loop_info _loop_info;

  std::unordered_map<std::string, std::vector<Parsed<v2::MultiStepEvent>>> _interactions;
  std::unordered_map<std::string, std::vector<reward::outcome_event>> _outcomes;
  std::vector<reward::outcome_event> _episodic_outcomes;

  std::deque<std::string> _order;
  std::deque<float> _rewards;

  bool _sorted = false;

  metrics::joiner_metrics _joiner_metrics;

  bool _current_je_is_skip_learn;

  bool _binary_to_json = false;
  std::ofstream _outfile;
};
