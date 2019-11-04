#pragma once
#include "trace_logger.h"
#include "rl.net.live_model.h"

namespace rl_net_native {
  template<typename ContextType>
  class binding_tracer : public reinforcement_learning::i_trace {
  public:
    // Inherited via i_trace
    binding_tracer(ContextType& _context)
      : context(_context) {
    }

    void binding_tracer::log(int log_level, const std::string& msg) override {
      if (context.trace_logger_callback != nullptr) {
        context.trace_logger_callback(log_level, msg.c_str());
      }
    }
  private:
    ContextType& context;
  };
}
