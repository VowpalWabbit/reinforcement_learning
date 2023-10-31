#include "console_tracer.h"

#include "str_util.h"

#include <iostream>

namespace reinforcement_learning
{
void console_tracer::log(int log_level, const std::string& msg)
{
  if (log_level < _log_level) { return; }
  std::cout << details::get_log_level_string(log_level) << ": " << msg << std::endl;
}

void console_tracer::set_level(int log_level) { _log_level = log_level; }
}  // namespace reinforcement_learning
