#include "binding_tracer.h"

namespace rl_net_native
{
binding_tracer::binding_tracer(livemodel_context& _context) : context(_context) {}

void binding_tracer::log(int log_level, const std::string& msg)
{
  if (context.trace_logger_callback != nullptr) { context.trace_logger_callback(log_level, msg.c_str()); }
}
}  // namespace rl_net_native
