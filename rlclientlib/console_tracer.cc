#include "console_tracer.h"

#include "str_util.h"

#include <iostream>

namespace reinforcement_learning
{
void console_tracer::log(int log_level, const std::string& msg)
{
  std::cout << get_log_level_string(log_level) << ": " << msg << std::endl;
}
}  // namespace reinforcement_learning
