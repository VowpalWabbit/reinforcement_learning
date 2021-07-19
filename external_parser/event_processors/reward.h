#pragma once

#include "metadata.h"

namespace reward {

struct outcome_event {
  outcome_event()
      : metadata({}), s_index(""), index(-1), s_value(""), value(0),
        enqueued_time_utc(TimePoint()), action_taken(false) {}
  metadata::event_metadata_info metadata;
  std::string s_index;
  int index;
  std::string s_value;
  float value;
  TimePoint enqueued_time_utc;
  bool action_taken;
};

using RewardFunctionType =
    float (*)(const std::vector<outcome_event> &);

inline float
average(const std::vector<outcome_event> &outcome_events) {
  float sum = 0.f;
  for (const auto &o : outcome_events) {
    sum += o.value;
  }

  return sum / outcome_events.size();
}

inline float
sum(const std::vector<outcome_event> &outcome_events) {
  float sum = 0.f;
  for (const auto &o : outcome_events) {
    sum += o.value;
  }

  return sum;
}

inline float
min(const std::vector<outcome_event> &outcome_events) {
  float min_reward = std::numeric_limits<float>::max();
  for (const auto &o : outcome_events) {
    if (o.value < min_reward) {
      min_reward = o.value;
    }
  }
  return min_reward;
}

inline float
max(const std::vector<outcome_event> &outcome_events) {
  float max_reward = std::numeric_limits<float>::min();
  for (const auto &o : outcome_events) {
    if (o.value > max_reward) {
      max_reward = o.value;
    }
  }
  return max_reward;
}

inline float
median(const std::vector<outcome_event> &outcome_events) {
  std::vector<float> values;
  for (const auto &o : outcome_events) {
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

inline float
earliest(const std::vector<outcome_event> &outcome_events) {
  auto oldest_valid_observation = TimePoint::max();
  float earliest_reward = 0.f;

  for (const auto &o : outcome_events) {
    if (o.enqueued_time_utc < oldest_valid_observation) {
      oldest_valid_observation = o.enqueued_time_utc;
      earliest_reward = o.value;
    }
  }

  return earliest_reward;
}
} // namespace reward
