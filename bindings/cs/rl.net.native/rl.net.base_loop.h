#pragma once

#include "rl.net.factory_context.h"

namespace rl_net_native
{
namespace constants
{
const char* const BINDING_TRACE_LOGGER = "BINDING_TRACE_LOGGER";
}

typedef void (*trace_logger_callback_t)(int log_level, const char* msg);
}  // namespace rl_net_native

typedef struct base_loop_context
{
  // callback funtion to user when there is background error.
  rl_net_native::background_error_callback_t background_error_callback;
  // callback funtion to user for trace log.
  rl_net_native::trace_logger_callback_t trace_logger_callback;
  // A trace log factory instance holder of one loop instance for binding calls.
  reinforcement_learning::trace_logger_factory_t* trace_logger_factory;
} base_loop_context_t;