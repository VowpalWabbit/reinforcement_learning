#pragma once
#include "rl.net.live_model.h"
#include "trace_logger.h"

namespace rl_net_native
{
class binding_tracer : public reinforcement_learning::i_trace
{
public:
  // Inherited via i_trace
  binding_tracer(livemodel_context& _context);
  void log(int log_level, const std::string& msg) override;

private:
  livemodel_context& context;
};
}  // namespace rl_net_native
