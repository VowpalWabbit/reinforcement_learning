#pragma once
#include "trace_logger.h"

namespace reinforcement_learning {
  class binding_tracer : public i_trace {
  public:
    // Inherited via i_trace
    binding_tracer(trace_callback &callback);
    void log(int log_level, const std::string &msg) override;
  private:
    trace_callback callback;
  };
}
