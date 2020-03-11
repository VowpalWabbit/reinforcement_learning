#pragma once

#include "rl.net.native.h"

namespace rl_net_native {
  namespace constants {
    const char *const BINDING_TRACE_LOGGER = "BINDING_TRACE_LOGGER";
  }

  using background_error_callback_t = void(*)(const reinforcement_learning::api_status&);
  using trace_logger_callback_t = void(*)(int log_level, const char* msg);
}

