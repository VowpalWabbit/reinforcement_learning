#include "console_tracer.h"
#include <iostream>
#include "str_util.h"

namespace reinforcement_learning {
  console_tracer::console_tracer(int _min_level) 
    : min_level(_min_level) {
  }
  void console_tracer::log(int log_level, const std::string& msg) {
    if (log_level >= min_level) {
      std::unique_lock<std::mutex> lock(_mutex);
      std::cout << get_log_level_string(log_level) << ": " << msg << std::endl;
    }
  }
}
