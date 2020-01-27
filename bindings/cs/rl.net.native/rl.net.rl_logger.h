#pragma once

#include "rl.net.callback.h"
#include "rl.net.native.h"

typedef struct rl_logger_context {
  // reinforcement learning live_model instance.
  reinforcement_learning::rl_logger* logger;
  // callback funtion to user when there is background error.
  rl_net_native::background_error_callback_t background_error_callback;
  // callback funtion to user for trace log.
  rl_net_native::trace_logger_callback_t trace_logger_callback;
  // A trace log factory instance holder of one live_model instance for binding calls.
  reinforcement_learning::trace_logger_factory_t* trace_logger_factory;
} rl_logger_context_t;

// Global exports
extern "C" {
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API rl_logger_context_t* CreateRlLogger(reinforcement_learning::utility::configuration* config);
  API void DeleteRlLogger(rl_logger_context_t* context);

  API int RlLoggerInit(rl_logger_context_t* context, reinforcement_learning::api_status* status = nullptr);

  API int RlLoggerLogCbInteraction(rl_logger_context_t* context, const char * context_json, const reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status = nullptr);
  API int RlLoggerLogOutcomeJson(rl_logger_context_t* context, const char * event_id,  const char* outcomeJson, reinforcement_learning::api_status* status = nullptr);
  API int RlLoggerLogOutcomeF(rl_logger_context_t* context, const char * event_id, float outcome, reinforcement_learning::api_status* status = nullptr);
  
  API void RRllLoggerSetCallback(rl_logger_context_t* context, rl_net_native::background_error_callback_t callback = nullptr);
  API void CbLoggerSetTrace(rl_logger_context_t* context, rl_net_native::trace_logger_callback_t trace_logger_callback = nullptr);
}
