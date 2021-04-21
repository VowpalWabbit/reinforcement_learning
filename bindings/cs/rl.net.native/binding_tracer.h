#pragma once
#include "trace_logger.h"
#include "rl.net.live_model.h"

namespace rl_net_native {
  class binding_tracer : public reinforcement_learning::i_trace {
  public:
    // Inherited via i_trace
    binding_tracer(livemodel_context& _context);
    void log(int log_level, const std::string &msg) override;
	virtual ~binding_tracer();
  private:
    livemodel_context& context;
  };
}
