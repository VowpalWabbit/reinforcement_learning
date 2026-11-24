#include "binding_tracer.h"

namespace rl_net_native
{
binding_tracer::binding_tracer(loop_context& _context) : context(_context) {}

void binding_tracer::log(int log_level, const std::string& msg)
{
  if (log_level < this->log_level) { return; }
  if (context.trace_logger_callback != nullptr) { context.trace_logger_callback(log_level, msg.c_str()); }
}

void binding_tracer::set_level(int log_level) { this->log_level = log_level; }

}  // namespace rl_net_native
