#pragma once

#include "constants.h"
#include "rl.net.factory_context.h"
#include "rl.net.loop_context.h"
#include "rl.net.native.h"

typedef struct slates_loop_context
{
  // reinforcement learning slates_loop instance.
  reinforcement_learning::slates_loop* slates_loop;
  // contains base fields for all loops
  loop_context_t loop_context;
} slates_loop_context_t;

// Global exports
extern "C"
{
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API slates_loop_context_t* CreateSlatesLoop(
      reinforcement_learning::utility::configuration* config, factory_context_t* factory_context);
  API void DeleteSlatesLoop(slates_loop_context_t* context);

  API int SlatesLoopInit(slates_loop_context_t* slates_loop, reinforcement_learning::api_status* status = nullptr);

  API int SlatesLoopRequestMultiSlotDecision(slates_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, reinforcement_learning::multi_slot_response* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int SlatesLoopRequestMultiSlotDecisionWithFlags(slates_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response* resp, reinforcement_learning::api_status* status = nullptr);
  API int SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlags(slates_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response* resp, const int* baseline_actions,
      const size_t baseline_actions_size, reinforcement_learning::api_status* status = nullptr);

  API int SlatesLoopRequestMultiSlotDecisionDetailed(slates_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, reinforcement_learning::multi_slot_response_detailed* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int SlatesLoopRequestMultiSlotDecisionDetailedWithFlags(slates_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response_detailed* resp, reinforcement_learning::api_status* status = nullptr);
  API int SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(slates_loop_context_t* context,
      const char* event_id, const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response_detailed* resp, const int* baseline_actions,
      size_t baseline_actions_size, reinforcement_learning::api_status* status = nullptr);

  API int SlatesLoopReportActionTaken(
      slates_loop_context_t* slates_loop, const char* event_id, reinforcement_learning::api_status* status = nullptr);
  API int SlatesLoopReportActionMultiIdTaken(slates_loop_context_t* slates_loop, const char* primary_id,
      const char* secondary_id, reinforcement_learning::api_status* status = nullptr);

  API int SlatesLoopReportOutcomeF(slates_loop_context_t* slates_loop, const char* event_id, float outcome,
      reinforcement_learning::api_status* status = nullptr);
  API int SlatesLoopReportOutcomeJson(slates_loop_context_t* slates_loop, const char* event_id, const char* outcomeJson,
      reinforcement_learning::api_status* status = nullptr);

  API int SlatesLoopRefreshModel(slates_loop_context_t* context, reinforcement_learning::api_status* status = nullptr);

  API void SlatesLoopSetCallback(
      slates_loop_context_t* slates_loop, rl_net_native::background_error_callback_t callback = nullptr);
  API void SlatesLoopSetTrace(
      slates_loop_context_t* slates_loop, rl_net_native::trace_logger_callback_t trace_logger_callback = nullptr);
}
