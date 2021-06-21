#pragma once

#include "joined_event.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace reward {

using RewardFunctionType = float (*)(const std::vector<joined_event::outcome_event> &);

inline float average(const std::vector<joined_event::outcome_event> &outcome_events) {
  float sum = 0.f;
  for (const auto &o : outcome_events) {
    sum += o.value;
  }

  return sum / outcome_events.size();
}

inline float sum(const std::vector<joined_event::outcome_event> &outcome_events) {
  float sum = 0.f;
  for (const auto &o : outcome_events) {
    sum += o.value;
  }

  return sum;
}

inline float min(const std::vector<joined_event::outcome_event> &outcome_events) {
  float min_reward = std::numeric_limits<float>::max();
  for (const auto &o : outcome_events) {
    if (o.value < min_reward) {
      min_reward = o.value;
    }
  }
  return min_reward;
}

inline float max(const std::vector<joined_event::outcome_event> &outcome_events) {
  float max_reward = std::numeric_limits<float>::min();
  for (const auto &o : outcome_events) {
    if (o.value > max_reward) {
      max_reward = o.value;
    }
  }
  return max_reward;
}

inline float median(const std::vector<joined_event::outcome_event> &outcome_events) {
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

inline float earliest(const std::vector<joined_event::outcome_event> &outcome_events) {
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
