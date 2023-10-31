#pragma once
#include "trace_logger.h"

namespace reinforcement_learning
{
class console_tracer : public i_trace
{
public:
  // Inherited via i_trace
  void log(int log_level, const std::string& msg) override;
#ifdef ENABLE_LOG_FILTERING
  void set_level(int log_level) override;
#endif

private:
#ifdef ENABLE_LOG_FILTERING
  int _log_level;
#endif
};
}  // namespace reinforcement_learning
