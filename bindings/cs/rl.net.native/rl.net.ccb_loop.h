#pragma once

#include "constants.h"
#include "rl.net.loop_context.h"
#include "rl.net.factory_context.h"
#include "rl.net.native.h"

typedef struct ccb_loop_context
{
  // reinforcement learning ccb_loop instance.
  reinforcement_learning::ccb_loop* ccb_loop;
  // contains base fields for all loops
  loop_context_t loop_context;
} ccb_loop_context_t;

// Global exports
extern "C"
{
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API ccb_loop_context_t* CreateCCBLoop(
      reinforcement_learning::utility::configuration* config, factory_context_t* factory_context);
  API void DeleteCCBLoop(ccb_loop_context_t* context);

  API int CCBLoopInit(ccb_loop_context_t* ccb_loop, reinforcement_learning::api_status* status = nullptr);

  API int CCBLoopRequestDecision(ccb_loop_context_t* ccb_loop, const char* context_json, int context_json_size,
      reinforcement_learning::decision_response* resp, reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopRequestDecisionWithFlags(ccb_loop_context_t* ccb_loop, const char* context_json,
      int context_json_size, unsigned int flags, reinforcement_learning::decision_response* resp,
      reinforcement_learning::api_status* status = nullptr);

  API int CCBLoopRequestMultiSlotDecision(ccb_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, reinforcement_learning::multi_slot_response* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopRequestMultiSlotDecisionWithFlags(ccb_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response* resp, reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopRequestMultiSlotDecisionWithBaselineAndFlags(ccb_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response* resp, const int* baseline_actions,
      const size_t baseline_actions_size, reinforcement_learning::api_status* status = nullptr);

  API int CCBLoopRequestMultiSlotDecisionDetailed(ccb_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, reinforcement_learning::multi_slot_response_detailed* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopRequestMultiSlotDecisionDetailedWithFlags(ccb_loop_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response_detailed* resp, reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(ccb_loop_context_t* context,
      const char* event_id, const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response_detailed* resp, const int* baseline_actions,
      size_t baseline_actions_size, reinforcement_learning::api_status* status = nullptr);

  API int CCBLoopReportActionTaken(
      ccb_loop_context_t* ccb_loop, const char* event_id, reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopReportActionMultiIdTaken(ccb_loop_context_t* ccb_loop, const char* primary_id,
      const char* secondary_id, reinforcement_learning::api_status* status = nullptr);

  API int CCBLoopReportOutcomeSlotF(ccb_loop_context_t* context, const char* event_id, unsigned int slot_index,
      float outcome, reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopReportOutcomeSlotJson(ccb_loop_context_t* context, const char* event_id, unsigned int slot_index,
      const char* outcome_json, reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopReportOutcomeSlotStringIdF(ccb_loop_context_t* context, const char* event_id, const char* slot_id,
      float outcome, reinforcement_learning::api_status* status = nullptr);
  API int CCBLoopReportOutcomeSlotStringIdJson(ccb_loop_context_t* context, const char* event_id,
      const char* slot_id, const char* outcome_json, reinforcement_learning::api_status* status = nullptr);

  API int CCBLoopRefreshModel(ccb_loop_context_t* context, reinforcement_learning::api_status* status = nullptr);

  API void CCBLoopSetCallback(
      ccb_loop_context_t* ccb_loop, rl_net_native::background_error_callback_t callback = nullptr);
  API void CCBLoopSetTrace(
      ccb_loop_context_t* ccb_loop, rl_net_native::trace_logger_callback_t trace_logger_callback = nullptr);
}
