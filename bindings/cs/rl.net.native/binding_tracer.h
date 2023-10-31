#pragma once
#include "rl.net.base_loop.h"
#include "trace_logger.h"

namespace rl_net_native
{
class binding_tracer : public reinforcement_learning::i_trace
{
public:
  // Inherited via i_trace
  binding_tracer(base_loop_context& _context);
  void log(int log_level, const std::string& msg) override;
  void set_level(int log_level) override;

private:
  base_loop_context& context;
  int log_level = 0;
};
}  // namespace rl_net_native
