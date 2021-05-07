#pragma once

#include "err_constants.h"

#include "example.h"
#include "generated/v2/MultiStepEvent_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "i_example_joiner.h"
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

class multistep_example_joiner : public i_example_joiner {
public:
  multistep_example_joiner(vw *vw); // TODO rule of 5

  virtual ~multistep_example_joiner();

  virtual void set_reward_function(const v2::RewardFunctionType type);
  virtual void set_default_reward(float default_reward);
  virtual void set_learning_mode_config(const v2::LearningModeType& learning_mode);
  virtual void set_problem_type_config(const v2::ProblemType& problem_type);

  // Takes an event which will have a timestamp and event payload
  // groups all events interactions with their event observations based on their
  // id. The grouped events can be processed when process_joined() is called
  virtual int process_event(const v2::JoinedEvent &joined_event);
  // Takes all grouped events, processes them (e.g. decompression) and populates
  // the examples array with complete example(s) ready to be used by vw for
  // training
  virtual int process_joined(v_array<example *> &examples);
  // true if there are still event-groups to be processed from a deserialized
  // batch
  virtual bool processing_batch();

  virtual void on_new_batch();

private:
  void populate_order();

private:
  template<typename event_t>
  struct Parsed {
    const v2::Metadata& meta;
    const event_t& event;
  };

private:
  std::vector<example *> _example_pool;

  vw *_vw;
  flatbuffers::DetachedBuffer _detached_buffer;

  float _default_reward = 0.f;
  RewardCalcType _reward_calculation;

  v2::LearningModeType _learning_mode_config = v2::LearningModeType_Online;
  v2::ProblemType _problem_type_config = v2::ProblemType_UNKNOWN;

  std::unordered_map<std::string, std::vector<Parsed<v2::MultiStepEvent>>> _interactions;
  std::unordered_map<std::string, std::vector<Parsed<v2::OutcomeEvent>>> _outcomes;
  std::vector<Parsed<v2::OutcomeEvent>> _episodic_outcomes;

  std::queue<std::string> _order;

  bool _sorted = false;
};
