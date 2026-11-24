#pragma once

#include "constants.h"
#include "rl.net.factory_context.h"
#include "rl.net.loop_context.h"
#include "rl.net.native.h"

typedef struct ca_loop_context
{
  // reinforcement learning ca_loop instance.
  reinforcement_learning::ca_loop* ca_loop;
  // contains base fields for all loops
  loop_context_t loop_context;
} ca_loop_context_t;

// Global exports
extern "C"
{
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API ca_loop_context_t* CreateCALoop(
      reinforcement_learning::utility::configuration* config, factory_context_t* factory_context);
  API void DeleteCALoop(ca_loop_context_t* context);

  API int CALoopInit(ca_loop_context_t* ca_loop, reinforcement_learning::api_status* status = nullptr);

  API int CALoopRequestContinuousAction(ca_loop_context_t* ca_loop, const char* event_id, const char* context_json,
      int context_json_size, reinforcement_learning::continuous_action_response* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int CALoopRequestContinuousActionWithFlags(ca_loop_context_t* ca_loop, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::continuous_action_response* resp, reinforcement_learning::api_status* status = nullptr);

  API int CALoopReportActionTaken(
      ca_loop_context_t* ca_loop, const char* event_id, reinforcement_learning::api_status* status = nullptr);
  API int CALoopReportActionMultiIdTaken(ca_loop_context_t* ca_loop, const char* primary_id, const char* secondary_id,
      reinforcement_learning::api_status* status = nullptr);

  API int CALoopReportOutcomeF(ca_loop_context_t* ca_loop, const char* event_id, float outcome,
      reinforcement_learning::api_status* status = nullptr);
  API int CALoopReportOutcomeJson(ca_loop_context_t* ca_loop, const char* event_id, const char* outcomeJson,
      reinforcement_learning::api_status* status = nullptr);

  API int CALoopRefreshModel(ca_loop_context_t* context, reinforcement_learning::api_status* status = nullptr);

  API void CALoopSetCallback(ca_loop_context_t* ca_loop, rl_net_native::background_error_callback_t callback = nullptr);
  API void CALoopSetTrace(
      ca_loop_context_t* ca_loop, rl_net_native::trace_logger_callback_t trace_logger_callback = nullptr);
}
