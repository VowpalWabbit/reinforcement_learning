#pragma once

#include "rl.net.callback.h"
#include "rl.net.native.h"

typedef struct cb_logger_context {
  // reinforcement learning live_model instance.
  reinforcement_learning::cb::logger* logger;
  // callback funtion to user when there is background error.
  rl_net_native::background_error_callback_t background_error_callback;
  // callback funtion to user for trace log.
  rl_net_native::trace_logger_callback_t trace_logger_callback;
  // A trace log factory instance holder of one live_model instance for binding calls.
  reinforcement_learning::trace_logger_factory_t* trace_logger_factory;
} cb_logger_context_t;

// Global exports
extern "C" {
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API cb_logger_context_t* CreateCbLogger(reinforcement_learning::utility::configuration* config);
  API void DeleteCbLogger(cb_logger_context_t* context);

  API int CbLoggerInit(cb_logger_context_t* context, reinforcement_learning::api_status* status = nullptr);

  API int CbLoggerLogF(cb_logger_context_t* context, const char * event_id, const char * context_json, const reinforcement_learning::ranking_response* resp, float outcome, reinforcement_learning::api_status* status = nullptr);
  API int CbLoggerLogJson(cb_logger_context_t* context, const char * event_id, const char * context_json, const reinforcement_learning::ranking_response* resp, const char* outcomeJson, reinforcement_learning::api_status* status = nullptr);

  API void CbLoggerSetCallback(cb_logger_context_t* context, rl_net_native::background_error_callback_t callback = nullptr);
  API void CbLoggerSetTrace(cb_logger_context_t* context, rl_net_native::trace_logger_callback_t trace_logger_callback = nullptr);
}
