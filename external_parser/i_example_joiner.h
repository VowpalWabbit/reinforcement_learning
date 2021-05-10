#pragma once

#include "err_constants.h"

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
// clang-format on

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

struct metadata_info {
  TimePoint client_time_utc;
  std::string app_id;
  v2::PayloadType payload_type;
  float pass_probability;
  v2::EventEncoding event_encoding;
  v2::LearningModeType learning_mode;
};

struct outcome_event {
  metadata_info metadata;
  std::string s_index;
  int index;
  std::string s_value;
  float value;
  TimePoint enqueued_time_utc;
};

struct joined_event {
  std::string joined_event_timestamp;
  metadata_info interaction_metadata;
  DecisionServiceInteraction interaction_data;
  std::vector<outcome_event> outcome_events;
  // Default Baseline Action for CB is 1 (rl client recommended actions are 1
  // indexed in the CB case)
  static const int baseline_action = 1;
};

using RewardCalcType = float (*)(const joined_event &);

namespace RewardFunctions {
float average(const joined_event &event);
float sum(const joined_event &event);
float min(const joined_event &event);
float max(const joined_event &event);
float median(const joined_event &event);
float apprentice(const joined_event &event);
float earliest(const joined_event &event);
} // namespace RewardFunctions

class i_example_joiner {
public:
  virtual ~i_example_joiner() = default;

  virtual void set_reward_function(const v2::RewardFunctionType type) = 0;
  virtual void set_default_reward(float default_reward) = 0;
  virtual void set_learning_mode_config(const v2::LearningModeType& learning_mode) = 0;
  virtual void set_problem_type_config(const v2::ProblemType& problem_type) = 0;

  // Takes an event which will have a timestamp and event payload
  // groups all events interactions with their event observations based on their
  // id. The grouped events can be processed when process_joined() is called
  virtual int process_event(const v2::JoinedEvent &joined_event) = 0;
  // Takes all grouped events, processes them (e.g. decompression) and populates
  // the examples array with complete example(s) ready to be used by vw for
  // training
  virtual int process_joined(v_array<example *> &examples) = 0;
  // true if there are still event-groups to be processed from a deserialized
  // batch
  virtual bool processing_batch() = 0;

  virtual void on_new_batch() = 0;
};
