#pragma once

#include "event_processors/joined_event.h"
#include "event_processors/loop.h"
#include "example.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "generated/v2/MultiStepEvent_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "joiners/i_joiner.h"
#include "v_array.h"
#include "metrics/metrics.h"

#include <list>
#include <queue>
#include <unordered_map>
// VW headers
// vw.h has to come before json_utils.h
// clang-format off
#include "vw.h"
#include "json_utils.h"
// clang-format on

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

class multistep_example_joiner : public i_joiner {
public:
  multistep_example_joiner(vw *vw); // TODO rule of 5

  virtual ~multistep_example_joiner();

  void set_reward_function(const v2::RewardFunctionType type, bool sticky) override;
  void set_default_reward(float default_reward, bool sticky) override;
  void set_learning_mode_config(v2::LearningModeType learning_mode, bool sticky) override;
  void set_problem_type_config(v2::ProblemType problem_type, bool sticky) override;
  bool joiner_ready() override;

  bool current_event_is_skip_learn() override;

  bool process_event(const v2::JoinedEvent &joined_event) override;
  bool process_joined(v_array<example *> &examples) override;
  bool processing_batch() override;

  void on_new_batch() override;
  void on_batch_read() override;
  metrics::joiner_metrics get_metrics() override;

private:
  template <typename event_t> struct Parsed {
    const TimePoint timestamp;
    const v2::Metadata &meta;
    const event_t &event;
  };

private:
  void populate_order();
  reward::outcome_event
  process_outcome(const Parsed<v2::OutcomeEvent> &event_meta);
  joined_event::joined_event
  process_interaction(const Parsed<v2::MultiStepEvent> &event_meta,
                      v_array<example *> &examples);

private:
  std::vector<example *> _example_pool;

  vw *_vw;
  flatbuffers::DetachedBuffer _detached_buffer;

  loop::sticky_value<reward::RewardFunctionType> _reward_calculation;
  loop::loop_info _loop_info;

  std::unordered_map<std::string, std::vector<Parsed<v2::MultiStepEvent>>>
      _interactions;
  std::unordered_map<std::string, std::vector<Parsed<v2::OutcomeEvent>>>
      _outcomes;
  std::vector<Parsed<v2::OutcomeEvent>> _episodic_outcomes;

  std::queue<std::string> _order;

  bool _sorted = false;

  metrics::joiner_metrics _joiner_metrics;

  bool _current_je_is_skip_learn;
};
