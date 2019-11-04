#pragma once

#include "rl.net.native.h"

namespace rl_net_native {
  namespace constants {
    const char *const BINDING_TRACE_LOGGER = "BINDING_TRACE_LOGGER";
  }
  typedef void(*background_error_callback_t)(const reinforcement_learning::api_status&);
  typedef void(*trace_logger_callback_t)(int log_level, const char* msg);
}

