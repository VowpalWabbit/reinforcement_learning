#pragma once

#include "api_status.h"
#include "constants.h"
#include "err_constants.h"
#include "future_compat.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "joined_log_provider.h"
#include "logger/message_type.h"
#include "logger/preamble.h"
#include "rl_string_view.h"
#include "sender.h"
#include "time_helper.h"
#include "vw/common/text_utils.h"
#include "vw/io/io_adapter.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <ratio>
#include <string>
#include <vector>

namespace reinforcement_learning
{
inline int parse_int(reinforcement_learning::string_view s, int& out, reinforcement_learning::api_status* status)
{
  // can't use stol because that throws an exception. Use strtol instead.
  char* end = nullptr;
  int i = strtol(s.data(), &end, 10);
  if (end <= s.data() && s.size() > 0)
  {
    out = 0;
    RETURN_ERROR_ARG(nullptr, status, invalid_argument, "invalid int");
  }
  out = i;
  return 0;
}

inline int parse_eud(
    reinforcement_learning::string_view eud_str, std::chrono::seconds& out, reinforcement_learning::api_status* status)
{
  std::vector<std::string> components;
  VW::tokenize(':', eud_str, components);
  if (components.size() != 3 || std::any_of(components.begin(), components.end(), [](const std::string& component) {
        return component.empty() || !std::all_of(component.begin(), component.end(), ::isdigit);
      }))
  { RETURN_ERROR_ARG(nullptr, status, invalid_argument, "invalid format of eud duration"); }

  int hours{};
  RETURN_IF_FAIL(parse_int(components[0], hours, status));

  int minutes{};
  RETURN_IF_FAIL(parse_int(components[1], minutes, status));

  int seconds{};
  RETURN_IF_FAIL(parse_int(components[2], seconds, status));

  out = std::chrono::hours(hours) + std::chrono::minutes(minutes) + std::chrono::seconds(seconds);
  return 0;
}

}  // namespace reinforcement_learning
