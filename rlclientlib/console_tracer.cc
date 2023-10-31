#include "console_tracer.h"

#include "str_util.h"

#include <iostream>

namespace reinforcement_learning
{
void console_tracer::log(int log_level, const std::string& msg)
{
#ifdef ENABLE_LOG_FILTERING
  if (log_level < _log_level) { return; }
#endif
  std::cout << details::get_log_level_string(log_level) << ": " << msg << std::endl;

}

#ifdef ENABLE_LOG_FILTERING
void console_tracer::set_level(int log_level)
{
  _log_level = log_level;
}
#endif
}  // namespace reinforcement_learning
