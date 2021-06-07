#pragma once

#include "error_constants.h"

#include "example.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "lru_dedup_cache.h"
#include "timestamp_helper.h"
#include "v_array.h"

#include <list>
#include <queue>
#include <unordered_map>
// VW headers
// vw.h has to come before json_utils.h
// clang-format off
#include "vw.h"
#include "json_utils.h"
#include "reward.h"
// clang-format on

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

class i_joiner {
public:
  virtual ~i_joiner() = default;

  virtual void set_reward_function(const v2::RewardFunctionType type) = 0;
  virtual void set_default_reward(float default_reward) = 0;
  virtual void set_learning_mode_config(v2::LearningModeType learning_mode) = 0;
  virtual void set_problem_type_config(v2::ProblemType problem_type) = 0;

  // Takes an event which will have a timestamp and event payload
  // groups all events interactions with their event observations based on their
  // id. The grouped events can be processed when process_joined() is called
  virtual bool process_event(const v2::JoinedEvent& joined_event) = 0;
  // Takes all grouped events, processes them (e.g. decompression) and populates
  // the examples array with complete example(s) ready to be used by vw for
  // training
  virtual bool process_joined(v_array<example*>& examples) = 0;
  // true if there are still event-groups to be processed from a deserialized
  // batch
  virtual bool processing_batch() = 0;

  virtual void on_new_batch() = 0;

  virtual void on_batch_read() = 0;
};
