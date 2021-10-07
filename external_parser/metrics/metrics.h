#pragma once

#include "event_processors/timestamp_helper.h"

namespace metrics {
struct joiner_metrics {
  size_t number_of_skipped_events = 0;
  float sum_cost_original = 0.f;
  TimePoint last_event_timestamp = TimePoint();
  TimePoint first_event_timestamp = TimePoint();
  std::string first_event_id = "";
  std::string last_event_id = "";
};
} // namespace metrics
