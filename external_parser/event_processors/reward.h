#pragma once

#include "generated/v2/OutcomeEvent_generated.h"
#include "metadata.h"

namespace reward
{
struct outcome_event
{
  outcome_event()
      : metadata({}), s_index(""), index(-1), s_value(""), value(0), enqueued_time_utc(TimePoint()), action_taken(false)
  {
  }
  metadata::event_metadata_info metadata;
  reinforcement_learning::messages::flatbuff::v2::IndexValue index_type;
  std::string s_index;
  int index;
  std::string s_value;
  float value;
  TimePoint enqueued_time_utc;
  bool action_taken;
};

using RewardFunctionType = float (*)(const std::vector<outcome_event>&, float default_reward);

inline float average(const std::vector<outcome_event>& outcome_events, float default_reward)
{
  float sum = 0.f;
  size_t N = 0;

  for (const auto& o : outcome_events)
  {
    if (!o.action_taken)
    {
      sum += o.value;
      N++;
    }
  }

  return N == 0 ? default_reward : sum / N;
}

inline float sum(const std::vector<outcome_event>& outcome_events, float default_reward)
{
  float sum = 0.f;
  size_t N = 0;

  for (const auto& o : outcome_events)
  {
    if (!o.action_taken)
    {
      sum += o.value;
      N++;
    }
  }

  return N == 0 ? default_reward : sum;
}

inline float min(const std::vector<outcome_event>& outcome_events, float default_reward)
{
  float min_reward = std::numeric_limits<float>::max();

  for (const auto& o : outcome_events)
  {
    if (!o.action_taken && o.value < min_reward) { min_reward = o.value; }
  }
  return min_reward == std::numeric_limits<float>::max() ? default_reward : min_reward;
}

inline float max(const std::vector<outcome_event>& outcome_events, float default_reward)
{
  float max_reward = std::numeric_limits<float>::min();

  for (const auto& o : outcome_events)
  {
    if (!o.action_taken && o.value > max_reward) { max_reward = o.value; }
  }
  return max_reward == std::numeric_limits<float>::min() ? default_reward : max_reward;
}

inline float median(const std::vector<outcome_event>& outcome_events, float default_reward)
{
  std::vector<float> values;
  for (const auto& o : outcome_events)
  {
    if (!o.action_taken) { values.push_back(o.value); }
  }

  auto outcome_events_size = values.size();

  if (outcome_events_size > 0)
  {
    sort(values.begin(), values.end());
    if (outcome_events_size % 2 == 0)
    { return (values[outcome_events_size / 2 - 1] + values[outcome_events_size / 2]) / 2; }
    else
    {
      return values[outcome_events_size / 2];
    }
  }

  return default_reward;
}

inline float earliest(const std::vector<outcome_event>& outcome_events, float default_reward)
{
  auto oldest_valid_observation = TimePoint::max();
  float earliest_reward = default_reward;

  for (const auto& o : outcome_events)
  {
    if (!o.action_taken && o.enqueued_time_utc < oldest_valid_observation)
    {
      oldest_valid_observation = o.enqueued_time_utc;
      earliest_reward = o.value;
    }
  }

  return earliest_reward;
}
}  // namespace reward
