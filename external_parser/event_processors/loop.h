#pragma once

// FileFormat_generated.h used for the payload type and encoding enum's
#include "generated/v2/FileFormat_generated.h"

#include "event_processors/timestamp_helper.h"
// VW headers
// vw.h has to come before json_utils.h
// clang-format off
#include "vw.h"
#include "json_utils.h"
// clang-format on

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

struct loop_info {
  float default_reward = 0.f;
  v2::RewardFunctionType type = v2::RewardFunctionType_Earliest;
  v2::LearningModeType learning_mode_config = v2::LearningModeType_Online;
  v2::ProblemType problem_type_config = v2::ProblemType_UNKNOWN;
};

struct metadata_info {
  TimePoint client_time_utc;
  std::string app_id;
  v2::PayloadType payload_type;
  float pass_probability;
  v2::EventEncoding event_encoding;
  std::string event_id;
  v2::LearningModeType learning_mode;
};

struct outcome_event {
  metadata_info metadata;
  std::string s_index;
  int index;
  std::string s_value;
  float value;
  TimePoint enqueued_time_utc;
  bool action_taken;
};

struct joined_event {
  joined_event(TimePoint &&tp, metadata_info &&mi,
               DecisionServiceInteraction &&id, std::string &&ctx,
               std::string &&mid)
      : joined_event_timestamp(std::move(tp)),
        interaction_metadata(std::move(mi)), interaction_data(std::move(id)),
        outcome_events({}), context(std::move(ctx)), model_id(std::move(mid)),
        ok(true) {}
  joined_event() : ok(true) {}

  TimePoint joined_event_timestamp;
  metadata_info interaction_metadata;
  DecisionServiceInteraction interaction_data;
  std::vector<outcome_event> outcome_events;
  std::string context;
  std::string model_id;
  bool ok; // ok till proved otherwise
  // Default Baseline Action for CB is 1 (rl client recommended actions are 1
  // indexed in the CB case)
  static const int baseline_action = 1;
};
