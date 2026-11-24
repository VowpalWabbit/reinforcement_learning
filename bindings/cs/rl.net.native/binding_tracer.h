#pragma once
#include "rl.net.loop_context.h"
#include "trace_logger.h"

namespace rl_net_native
{
class binding_tracer : public reinforcement_learning::i_trace
{
public:
  // Inherited via i_trace
  binding_tracer(loop_context& _context);
  void log(int log_level, const std::string& msg) override;
  void set_level(int log_level) override;

private:
  loop_context& context;
  int log_level = 0;
};
}  // namespace rl_net_native
