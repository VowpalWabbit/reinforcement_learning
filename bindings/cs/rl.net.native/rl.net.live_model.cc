#include "rl.net.live_model.h"
#include "binding_tracer.h"
#include "constants.h"
#include "err_constants.h"
#include "rl.net.live_model.h"
#include "trace_logger.h"

#include <iostream>

static void pipe_background_error_callback(const reinforcement_learning::api_status& status, livemodel_context_t* context)
{
  auto managed_backgroud_error_callback_local = context->background_error_callback;
  if (managed_backgroud_error_callback_local)
  {
    managed_backgroud_error_callback_local(status);
  }
}

API livemodel_context_t* CreateLiveModel(reinforcement_learning::utility::configuration* config, factory_context_t* factory_context)
{
  livemodel_context_t* context = new livemodel_context_t;
  context->background_error_callback = nullptr;
  context->trace_logger_callback = nullptr;
  context->trace_logger_factory = nullptr;

  // Create a trace log factory by passing in below creator. It allows LiveModel to use trace_logger provided by user.
  const auto binding_tracer_create = [context](reinforcement_learning::i_trace** retval,
    const reinforcement_learning::utility::configuration& cfg,
    reinforcement_learning::i_trace* trace_logger,
    reinforcement_learning::api_status* status)
  {
    *retval = new rl_net_native::binding_tracer(*context);
    return reinforcement_learning::error_code::success;
  };


  // TODO: Unify this factory projection and the sender_factory projection in FactoryContext.
  reinforcement_learning::trace_logger_factory_t* trace_logger_factory =
    new reinforcement_learning::trace_logger_factory_t(*factory_context->trace_logger_factory);

  // Register the type in factor to use trace logger creatation function.
  trace_logger_factory->register_type(rl_net_native::constants::BINDING_TRACE_LOGGER, binding_tracer_create);

  // This is a clone of cleanup_trace_logger_factory
  std::swap(trace_logger_factory, factory_context->trace_logger_factory);
  if (trace_logger_factory != nullptr &&
    trace_logger_factory != &reinforcement_learning::trace_logger_factory)
  {
    delete trace_logger_factory;
  }

  // Set TRACE_LOG_IMPLEMENTATION configuration to use trace logger.
  config->set(reinforcement_learning::name::TRACE_LOG_IMPLEMENTATION, rl_net_native::constants::BINDING_TRACE_LOGGER);

  context->livemodel = new reinforcement_learning::live_model(
    *config,
    pipe_background_error_callback,
    context,
    factory_context->trace_logger_factory,
    factory_context->data_transport_factory,
    factory_context->model_factory,
    factory_context->sender_factory,
    factory_context->time_provider_factory);

  return context;
}

API void DeleteLiveModel(livemodel_context_t* context)
{
  // Since the livemodel destructor waits for queues to drain, this can have unhappy consequences,
  // so detach the callback pipe first. This will cause all background callbacks to no-op in the
  // unmanaged side, which maintains expected thread semantics (the user of the bindings)
  context->background_error_callback = nullptr;
  context->trace_logger_callback = nullptr;

  delete context->trace_logger_factory;
  delete context->livemodel;
  delete context;
}

API int LiveModelInit(livemodel_context_t* context, reinforcement_learning::api_status* status)
{
  return context->livemodel->init(status);
}

API int LiveModelChooseRank(livemodel_context_t* context, const char* event_id, const char* context_json, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status)
{
  if (event_id == nullptr)
  {
    return context->livemodel->choose_rank(context_json, *resp, status);
  }

  return context->livemodel->choose_rank(event_id, context_json, *resp, status);
}

API int LiveModelChooseRankWithFlags(livemodel_context_t* context, const char* event_id, const char* context_json, unsigned int flags, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status)
{
  return context->livemodel->choose_rank(event_id, context_json, flags, *resp, status);
}

API int LiveModelRequestContinuousAction(livemodel_context_t* context, const char * event_id, const char * context_json, reinforcement_learning::continuous_action_response* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  return context->livemodel->request_continuous_action(event_id, context_json, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestContinuousActionWithFlags(livemodel_context_t* context, const char * event_id, const char * context_json, unsigned int flags, reinforcement_learning::continuous_action_response* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  return context->livemodel->request_continuous_action(context_json, flags, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestDecision(livemodel_context_t* context, const char* context_json, reinforcement_learning::decision_response* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  return context->livemodel->request_decision(context_json, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestDecisionWithFlags(livemodel_context_t* context, const char* context_json, unsigned int flags, reinforcement_learning::decision_response* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  return context->livemodel->request_decision(context_json, flags, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestMultiSlotDecision(livemodel_context_t* context, const char* event_id, const char* context_json, reinforcement_learning::multi_slot_response* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
    if (event_id == nullptr)
      return context->livemodel->request_multi_slot_decision(context_json, *resp, status);
    else
      return context->livemodel->request_multi_slot_decision(event_id, context_json, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestMultiSlotDecisionWithFlags(livemodel_context_t* context, const char* event_id, const char* context_json, unsigned int flags, reinforcement_learning::multi_slot_response* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
    if (event_id == nullptr)
      return context->livemodel->request_multi_slot_decision(context_json, flags, *resp, status);
    else
      return context->livemodel->request_multi_slot_decision(event_id, context_json, flags, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestMultiSlotDecisionWithBaseline(livemodel_context_t* context, const char* event_id, const char* context_json, reinforcement_learning::multi_slot_response* resp, const int* baseline_actions, size_t baseline_actions_size, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
    if (event_id == nullptr)
      return context->livemodel->request_multi_slot_decision(context_json, *resp, baseline_actions, baseline_actions_size, status);
    else
      return context->livemodel->request_multi_slot_decision(event_id, context_json, *resp, baseline_actions, baseline_actions_size, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestMultiSlotDecisionWithBaselineAndFlags(livemodel_context_t* context, const char* event_id, const char* context_json, unsigned int flags, reinforcement_learning::multi_slot_response* resp, const int* baseline_actions, size_t baseline_actions_size, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
    if (event_id == nullptr)
      return context->livemodel->request_multi_slot_decision(context_json, flags, *resp, baseline_actions, baseline_actions_size, status);
    else
      return context->livemodel->request_multi_slot_decision(event_id, context_json, flags, *resp, baseline_actions, baseline_actions_size, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestMultiSlotDecisionDetailed(livemodel_context_t* context, const char * event_id, const char * context_json, reinforcement_learning::multi_slot_response_detailed* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
    if (event_id == nullptr)
      return context->livemodel->request_multi_slot_decision(context_json, *resp, status);
    else
      return context->livemodel->request_multi_slot_decision(event_id, context_json, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestMultiSlotDecisionDetailedWithFlags(livemodel_context_t* context, const char * event_id, const char * context_json, unsigned int flags, reinforcement_learning::multi_slot_response_detailed* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
    if (event_id == nullptr)
      return context->livemodel->request_multi_slot_decision(context_json, flags, *resp, status);
    else
      return context->livemodel->request_multi_slot_decision(event_id, context_json, flags, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestMultiSlotDecisionDetailedWithBaseline(livemodel_context_t* context, const char * event_id, const char * context_json, reinforcement_learning::multi_slot_response_detailed* resp, const int* baseline_actions, size_t baseline_actions_size, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
    if (event_id == nullptr)
      return context->livemodel->request_multi_slot_decision(context_json, *resp, baseline_actions, baseline_actions_size, status);
    else
      return context->livemodel->request_multi_slot_decision(event_id, context_json, *resp, baseline_actions, baseline_actions_size, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int LiveModelRequestMultiSlotDecisionDetailedWithBaselineAndFlags(livemodel_context_t* context, const char * event_id, const char * context_json, unsigned int flags, reinforcement_learning::multi_slot_response_detailed* resp, const int* baseline_actions, size_t baseline_actions_size, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
    if (event_id == nullptr)
      return context->livemodel->request_multi_slot_decision(context_json, flags, *resp, baseline_actions, baseline_actions_size, status);
    else
      return context->livemodel->request_multi_slot_decision(event_id, context_json, flags, *resp, baseline_actions, baseline_actions_size, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}



API int LiveModelReportActionTaken(livemodel_context_t* context, const char* event_id, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_action_taken(event_id, status);
}

API int LiveModelReportOutcomeF(livemodel_context_t* context, const char* event_id, float outcome, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_outcome(event_id, outcome, status);
}

API int LiveModelReportOutcomeJson(livemodel_context_t* context, const char* event_id, const char* outcome_json, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_outcome(event_id, outcome_json, status);
}

API int LiveModelReportOutcomeSlotF(livemodel_context_t* context, const char* event_id, unsigned int slot_index, float outcome, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_outcome(event_id, slot_index, outcome, status);
}

API int LiveModelReportOutcomeSlotJson(livemodel_context_t* context, const char* event_id, unsigned int slot_index, const char* outcome_json, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_outcome(event_id, slot_index, outcome_json, status);
}

API int LiveModelReportOutcomeSlotStringIdF(livemodel_context_t* context, const char* event_id, const char* slot_id, float outcome, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_outcome(event_id, slot_id, outcome, status);
}

API int LiveModelReportOutcomeSlotStringIdJson(livemodel_context_t* context, const char* event_id, const char* slot_id, const char* outcome_json, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_outcome(event_id, slot_id, outcome_json, status);
}

API int LiveModelRefreshModel(livemodel_context_t* context, reinforcement_learning::api_status* status)
{
  return context->livemodel->refresh_model(status);
}

API void LiveModelSetCallback(livemodel_context_t* livemodel, rl_net_native::background_error_callback_t callback)
{
  livemodel->background_error_callback = callback;
}

API void LiveModelSetTrace(livemodel_context_t* livemodel, rl_net_native::trace_logger_callback_t callback)
{
  livemodel->trace_logger_callback = callback;
}
