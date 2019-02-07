#include "binding_tracer.h"

namespace rl_net_native {

  binding_tracer::binding_tracer(trace_logger_callback_t &callback) {
    this->callback = callback;
  }

  void binding_tracer::log(int log_level, const std::string& msg) {
    if (this->callback != nullptr) {
      this->callback(log_level, msg.c_str());
    }
  }
}
