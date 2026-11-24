#pragma once

#include "constants.h"
#include "rl.net.factory_context.h"
#include "rl.net.loop_context.h"
#include "rl.net.native.h"

typedef struct cb_loop_context
{
  // reinforcement learning cb_loop instance.
  reinforcement_learning::cb_loop* cb_loop;
  // contains base fields for all loops
  loop_context_t loop_context;
} cb_loop_context_t;

// Global exports
extern "C"
{
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API cb_loop_context_t* CreateCBLoop(
      reinforcement_learning::utility::configuration* config, factory_context_t* factory_context);
  API void DeleteCBLoop(cb_loop_context_t* context);

  API int CBLoopInit(cb_loop_context_t* cb_loop, reinforcement_learning::api_status* status = nullptr);

  API int CBLoopChooseRank(cb_loop_context_t* cb_loop, const char* event_id, const char* context_json,
      int context_json_size, reinforcement_learning::ranking_response* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int CBLoopChooseRankWithFlags(cb_loop_context_t* cb_loop, const char* event_id, const char* context_json,
      int context_json_size, unsigned int flags, reinforcement_learning::ranking_response* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int CBLoopReportActionTaken(
      cb_loop_context_t* cb_loop, const char* event_id, reinforcement_learning::api_status* status = nullptr);
  API int CBLoopReportActionMultiIdTaken(cb_loop_context_t* cb_loop, const char* primary_id, const char* secondary_id,
      reinforcement_learning::api_status* status = nullptr);

  API int CBLoopReportOutcomeF(cb_loop_context_t* cb_loop, const char* event_id, float outcome,
      reinforcement_learning::api_status* status = nullptr);
  API int CBLoopReportOutcomeJson(cb_loop_context_t* cb_loop, const char* event_id, const char* outcomeJson,
      reinforcement_learning::api_status* status = nullptr);

  API int CBLoopRefreshModel(cb_loop_context_t* context, reinforcement_learning::api_status* status = nullptr);

  API void CBLoopSetCallback(cb_loop_context_t* cb_loop, rl_net_native::background_error_callback_t callback = nullptr);
  API void CBLoopSetTrace(
      cb_loop_context_t* cb_loop, rl_net_native::trace_logger_callback_t trace_logger_callback = nullptr);
}
