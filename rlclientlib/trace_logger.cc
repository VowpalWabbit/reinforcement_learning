#include "trace_logger.h"

#include <algorithm>

const char* reinforcement_learning::details::get_log_level_string(int log_level)
{
  switch (log_level)
  {
    case reinforcement_learning::LEVEL_DEBUG:
      return reinforcement_learning::STR_LEVEL_DEBUG;
    case reinforcement_learning::LEVEL_INFO:
      return reinforcement_learning::STR_LEVEL_INFO;
    case reinforcement_learning::LEVEL_WARN:
      return reinforcement_learning::STR_LEVEL_WARN;
    case reinforcement_learning::LEVEL_ERROR:
      return reinforcement_learning::STR_LEVEL_ERROR;
    default:
      return "LOG";
  }
}

int reinforcement_learning::details::get_log_level_from_string(const std::string& level, int& level_value, api_status* status)
{
  // Convert level to an uppercase string for comparison
  std::string level_upper = level;
  std::transform(level_upper.begin(), level_upper.end(), level_upper.begin(), ::toupper);

  if (level_upper == reinforcement_learning::STR_LEVEL_DEBUG)
  {
    level_value = reinforcement_learning::LEVEL_DEBUG;
    return 0;
  }
  else if (level_upper == reinforcement_learning::STR_LEVEL_INFO)
  {
    level_value = reinforcement_learning::LEVEL_INFO;
    return 0;
  }
  else if (level_upper == reinforcement_learning::STR_LEVEL_WARN)
  {
    level_value = reinforcement_learning::LEVEL_WARN;
    return 0;
  }
  else if (level_upper == reinforcement_learning::STR_LEVEL_ERROR)
  {
    level_value = reinforcement_learning::LEVEL_ERROR;
    return 0;
  }
  else
  {
    // Check if level is actually a number and return it
    try
    {
      level_value = std::stoi(level);
      return 0;
    }
    catch (const std::invalid_argument&)
    {
      RETURN_ERROR_ARG(nullptr, status, invalid_argument, "Provided log level is an invalid string.");
    }
  }

}