#pragma once
#include "api_status.h"

#include <vector>
#include <utility>

namespace reinforcement_learning {
  class i_trace;
  namespace utility {
  int get_action_count(size_t& count, const char *context, i_trace* trace, api_status* status = nullptr);
  int get_event_ids(std::vector<std::pair<size_t, std::string>>& event_ids, const char *context, i_trace* trace, api_status* status);
  int get_decision_count(size_t& count, const char *context, i_trace* trace, api_status* status = nullptr);
}}
