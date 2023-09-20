#pragma once

#include "constants.h"
#include "rl.net.factory_context.h"
#include "rl.net.native.h"
#include "rl.net.base_loop.h"

typedef struct livemodel_context
{
  // reinforcement learning live_model instance.
  reinforcement_learning::live_model* livemodel;
  // contains base fields for all loops
  base_loop_context_t base_loop_context;
} livemodel_context_t;

// Global exports
extern "C"
{
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API livemodel_context_t* CreateLiveModel(
      reinforcement_learning::utility::configuration* config, factory_context_t* factory_context);
  API void DeleteLiveModel(livemodel_context_t* context);

  API int LiveModelInit(livemodel_context_t* livemodel, reinforcement_learning::api_status* status = nullptr);

  API int LiveModelChooseRank(livemodel_context_t* livemodel, const char* event_id, const char* context_json,
      int context_json_size, reinforcement_learning::ranking_response* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int LiveModelChooseRankWithFlags(livemodel_context_t* livemodel, const char* event_id, const char* context_json,
      int context_json_size, unsigned int flags, reinforcement_learning::ranking_response* resp,
      reinforcement_learning::api_status* status = nullptr);

  API int LiveModelRequestContinuousAction(livemodel_context_t* livemodel, const char* event_id,
      const char* context_json, int context_json_size, reinforcement_learning::continuous_action_response* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int LiveModelRequestContinuousActionWithFlags(livemodel_context_t* livemodel, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::continuous_action_response* resp, reinforcement_learning::api_status* status = nullptr);

  API int LiveModelRequestDecision(livemodel_context_t* livemodel, const char* context_json, int context_json_size,
      reinforcement_learning::decision_response* resp, reinforcement_learning::api_status* status = nullptr);
  API int LiveModelRequestDecisionWithFlags(livemodel_context_t* livemodel, const char* context_json,
      int context_json_size, unsigned int flags, reinforcement_learning::decision_response* resp,
      reinforcement_learning::api_status* status = nullptr);

  API int LiveModelRequestMultiSlotDecision(livemodel_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, reinforcement_learning::multi_slot_response* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int LiveModelRequestMultiSlotDecisionWithFlags(livemodel_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response* resp, reinforcement_learning::api_status* status = nullptr);
  API int LiveModelRequestMultiSlotDecisionWithBaselineAndFlags(livemodel_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response* resp, const int* baseline_actions,
      const size_t baseline_actions_size, reinforcement_learning::api_status* status = nullptr);

  API int LiveModelRequestMultiSlotDecisionDetailed(livemodel_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, reinforcement_learning::multi_slot_response_detailed* resp,
      reinforcement_learning::api_status* status = nullptr);
  API int LiveModelRequestMultiSlotDecisionDetailedWithFlags(livemodel_context_t* context, const char* event_id,
      const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response_detailed* resp, reinforcement_learning::api_status* status = nullptr);
  API int LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlags(livemodel_context_t* context,
      const char* event_id, const char* context_json, int context_json_size, unsigned int flags,
      reinforcement_learning::multi_slot_response_detailed* resp, const int* baseline_actions,
      size_t baseline_actions_size, reinforcement_learning::api_status* status = nullptr);
  API int LiveModelRequestEpisodicDecisionWithFlags(livemodel_context_t* context, const char* event_id,
      const char* previous_id, const char* context_json, unsigned int flags,
      reinforcement_learning::ranking_response& resp, reinforcement_learning::episode_state& episode,
      reinforcement_learning::api_status* status);
  API int LiveModelRequestEpisodicDecision(livemodel_context_t* context, const char* event_id, const char* previous_id,
      const char* context_json, reinforcement_learning::ranking_response& resp,
      reinforcement_learning::episode_state& episode, reinforcement_learning::api_status* status);

  API int LiveModelReportActionTaken(
      livemodel_context_t* livemodel, const char* event_id, reinforcement_learning::api_status* status = nullptr);
  API int LiveModelReportActionMultiIdTaken(livemodel_context_t* livemodel, const char* primary_id,
      const char* secondary_id, reinforcement_learning::api_status* status = nullptr);

  API int LiveModelReportOutcomeF(livemodel_context_t* livemodel, const char* event_id, float outcome,
      reinforcement_learning::api_status* status = nullptr);
  API int LiveModelReportOutcomeJson(livemodel_context_t* livemodel, const char* event_id, const char* outcomeJson,
      reinforcement_learning::api_status* status = nullptr);
  API int LiveModelReportOutcomeSlotF(livemodel_context_t* context, const char* event_id, unsigned int slot_index,
      float outcome, reinforcement_learning::api_status* status = nullptr);
  API int LiveModelReportOutcomeSlotJson(livemodel_context_t* context, const char* event_id, unsigned int slot_index,
      const char* outcome_json, reinforcement_learning::api_status* status = nullptr);
  API int LiveModelReportOutcomeSlotStringIdF(livemodel_context_t* context, const char* event_id, const char* slot_id,
      float outcome, reinforcement_learning::api_status* status = nullptr);
  API int LiveModelReportOutcomeSlotStringIdJson(livemodel_context_t* context, const char* event_id,
      const char* slot_id, const char* outcome_json, reinforcement_learning::api_status* status = nullptr);

  API int LiveModelRefreshModel(livemodel_context_t* context, reinforcement_learning::api_status* status = nullptr);

  API void LiveModelSetCallback(
      livemodel_context_t* livemodel, rl_net_native::background_error_callback_t callback = nullptr);
  API void LiveModelSetTrace(
      livemodel_context_t* livemodel, rl_net_native::trace_logger_callback_t trace_logger_callback = nullptr);
}
