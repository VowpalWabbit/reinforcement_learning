#pragma once

#include "err_constants.h"

#include "example.h"
#include "generated/v2/MultiStepEvent_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "i_joiner.h"
#include "v_array.h"

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

  void set_reward_function(const v2::RewardFunctionType type) override;
  void set_default_reward(float default_reward) override;
  void set_learning_mode_config(v2::LearningModeType learning_mode) override;
  void set_problem_type_config(v2::ProblemType problem_type) override;

  bool process_event(const v2::JoinedEvent &joined_event) override;
  bool process_joined(v_array<example *> &examples) override;
  bool processing_batch() override;

  void on_new_batch() override;

private:
  template<typename event_t>
  struct Parsed {
    const TimePoint timestamp;
    const v2::Metadata& meta;
    const event_t& event;
  };


private:
  void populate_order();
  outcome_event process_outcome(const Parsed<v2::OutcomeEvent> &event_meta);
  joined_event process_interaction(
    const Parsed<v2::MultiStepEvent> &event_meta,
    v_array<example *> &examples);

private:
  std::vector<example *> _example_pool;

  vw *_vw;
  flatbuffers::DetachedBuffer _detached_buffer;

  float _default_reward = 0.f;
  reward::RewardFunctionType  _reward_calculation;

  v2::LearningModeType _learning_mode_config = v2::LearningModeType_Online;
  v2::ProblemType _problem_type_config = v2::ProblemType_UNKNOWN;

  std::unordered_map<std::string, std::vector<Parsed<v2::MultiStepEvent>>> _interactions;
  std::unordered_map<std::string, std::vector<Parsed<v2::OutcomeEvent>>> _outcomes;
  std::vector<Parsed<v2::OutcomeEvent>> _episodic_outcomes;

  std::queue<std::string> _order;

  bool _sorted = false;
};
