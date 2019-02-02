#include "binding_tracer.h"

namespace reinforcement_learning {

  binding_tracer::binding_tracer(trace_callback &callback) {
    this->callback = callback;
  }

  void binding_tracer::log(int log_level, const std::string& msg) {
    if (this->callback != nullptr) {
      this->callback(log_level, msg.c_str());
    }
  }
}
