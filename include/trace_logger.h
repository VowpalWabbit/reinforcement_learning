#pragma once
#include "api_status.h"

#include <string>

namespace reinforcement_learning
{
const int LEVEL_DEBUG = -10;
const int LEVEL_INFO = 0;
const int LEVEL_WARN = 10;
const int LEVEL_ERROR = 20;

const char* const STR_LEVEL_DEBUG = "DEBUG";
const char* const STR_LEVEL_INFO = "INFO";
const char* const STR_LEVEL_WARN = "WARN";
const char* const STR_LEVEL_ERROR = "ERROR";

namespace details
{
const char* get_log_level_string(int log_level);
int get_log_level_from_string(const std::string& level, int& level_value, api_status* status);
}  // namespace details
}  // namespace reinforcement_learning

#define TRACE_LOG(logger, level, msg)                   \
  do {                                                  \
    if (logger != nullptr) { logger->log(level, msg); } \
  } while (0)

#define TRACE_DEBUG(logger, msg) TRACE_LOG(logger, reinforcement_learning::LEVEL_DEBUG, msg)
#define TRACE_INFO(logger, msg) TRACE_LOG(logger, reinforcement_learning::LEVEL_INFO, msg)
#define TRACE_WARN(logger, msg) TRACE_LOG(logger, reinforcement_learning::LEVEL_WARN, msg)
#define TRACE_ERROR(logger, msg) TRACE_LOG(logger, reinforcement_learning::LEVEL_ERROR, msg)

namespace reinforcement_learning
{
class i_trace
{
public:
  virtual void log(int log_level, const std::string& msg) = 0;
#ifdef ENABLE_LOG_FILTERING
  virtual void set_level(int log_level) = 0;
#endif
  virtual ~i_trace(){};
};
}  // namespace reinforcement_learning
