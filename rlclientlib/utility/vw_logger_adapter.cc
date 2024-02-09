#include "vw_logger_adapter.h"

#include "trace_logger.h"

static void vw_log_to_trace_logger(void* trace_logger, VW::io::log_level log_level, const std::string& msg)
{
  if (trace_logger == nullptr) { return; }
  auto* i_trace_ptr = static_cast<reinforcement_learning::i_trace*>(trace_logger);
  switch (log_level)
  {
    case VW::io::log_level::TRACE_LEVEL:
    case VW::io::log_level::DEBUG_LEVEL:
      TRACE_DEBUG(i_trace_ptr, msg);
      break;

    case VW::io::log_level::INFO_LEVEL:
      TRACE_INFO(i_trace_ptr, msg);
      break;

    case VW::io::log_level::WARN_LEVEL:
      TRACE_WARN(i_trace_ptr, msg);
      break;

    case VW::io::log_level::ERROR_LEVEL:
    case VW::io::log_level::CRITICAL_LEVEL:
      TRACE_ERROR(i_trace_ptr, msg);
      break;

    default:
      break;
  }
}

VW::io::logger reinforcement_learning::utility::make_vw_trace_logger(i_trace* trace_logger)
{
  return VW::io::create_custom_sink_logger(trace_logger, vw_log_to_trace_logger);
}
