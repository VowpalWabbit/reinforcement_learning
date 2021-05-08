#pragma once

// FileFormat_generated.h used for the payload type and encoding enum's
#include "generated/v2/FileFormat_generated.h"

#include "timestamp_helper.h"
// VW headers
#include "json_utils.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

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
  joined_event(const TimePoint &tp, metadata_info &&mi,
               DecisionServiceInteraction &&id, const std::string &ctx,
               const std::string &mid)
      : joined_event_timestamp(tp), interaction_metadata(std::move(mi)),
        interaction_data(std::move(id)), outcome_events({}), context(ctx),
        model_id(mid), ok(true) {}
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

namespace reward {

using RewardFunctionType = float (*)(const joined_event &);

inline float average(const joined_event &event) {
  float sum = 0.f;
  for (const auto &o : event.outcome_events) {
    sum += o.value;
  }

  return sum / event.outcome_events.size();
}

inline float sum(const joined_event &event) {
  float sum = 0.f;
  for (const auto &o : event.outcome_events) {
    sum += o.value;
  }

  return sum;
}

inline float min(const joined_event &event) {
  float min_reward = std::numeric_limits<float>::max();
  for (const auto &o : event.outcome_events) {
    if (o.value < min_reward) {
      min_reward = o.value;
    }
  }
  return min_reward;
}

inline float max(const joined_event &event) {
  float max_reward = std::numeric_limits<float>::min();
  for (const auto &o : event.outcome_events) {
    if (o.value > max_reward) {
      max_reward = o.value;
    }
  }
  return max_reward;
}

inline float median(const joined_event &event) {
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

inline float earliest(const joined_event &event) {
  auto oldest_valid_observation = TimePoint::max();
  float earliest_reward = 0.f;

  for (const auto &o : event.outcome_events) {
    if (o.enqueued_time_utc < oldest_valid_observation) {
      oldest_valid_observation = o.enqueued_time_utc;
      earliest_reward = o.value;
    }
  }

  return earliest_reward;
}
} // namespace reward
