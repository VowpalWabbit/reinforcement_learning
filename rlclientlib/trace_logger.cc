#include "trace_logger.h"
#include <map>

const std::map<int, const char* const> debug_levels = {
  {reinforcement_learning::LEVEL_DEBUG, reinforcement_learning::STR_LEVEL_DEBUG},
  {reinforcement_learning::LEVEL_INFO, reinforcement_learning::STR_LEVEL_INFO},
  {reinforcement_learning::LEVEL_WARN, reinforcement_learning::STR_LEVEL_WARN},
  {reinforcement_learning::LEVEL_ERROR, reinforcement_learning::STR_LEVEL_ERROR},
};

const char* get_log_level_string(int log_level) {
  const auto result = debug_levels.find(log_level);
  return result != debug_levels.end() ? result->second : "LOG";
}

int get_log_level(const char* log_level_str) {
  for (const auto& it : debug_levels) {
    if (std::strcmp(it.second, log_level_str) == 0) {
      return it.first;
    }
  }
  return reinforcement_learning::LEVEL_INFO;
}
