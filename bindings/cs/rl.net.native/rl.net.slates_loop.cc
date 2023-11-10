#include "rl.net.slates_loop.h"

#include "binding_tracer.h"
#include "constants.h"
#include "err_constants.h"
#include "trace_logger.h"

#include <iostream>

static void pipe_background_error_callback(
    const reinforcement_learning::api_status& status, slates_loop_context_t* context)
{
  auto managed_backgroud_error_callback_local = context->loop_context.background_error_callback;
  if (managed_backgroud_error_callback_local) { managed_backgroud_error_callback_local(status); }
}

API slates_loop_context_t* CreateSlatesLoop(
    reinforcement_learning::utility::configuration* config, factory_context_t* factory_context)
{
  slates_loop_context_t* context = new slates_loop_context_t;
  context->loop_context.background_error_callback = nullptr;
  context->loop_context.trace_logger_callback = nullptr;
  context->loop_context.trace_logger_factory = nullptr;

  // Create a trace log factory by passing in below creator. It allows SlatesLoop to use trace_logger provided by user.
  const auto binding_tracer_create = [context](std::unique_ptr<reinforcement_learning::i_trace>& retval,
                                         const reinforcement_learning::utility::configuration& cfg,
                                         reinforcement_learning::i_trace* trace_logger,
                                         reinforcement_learning::api_status* status)
  {
    retval.reset(new rl_net_native::binding_tracer(context->loop_context));
    return reinforcement_learning::error_code::success;
  };

  // TODO: Unify this factory projection and the sender_factory projection in FactoryContext.
  reinforcement_learning::trace_logger_factory_t* trace_logger_factory =
      new reinforcement_learning::trace_logger_factory_t(*factory_context->trace_logger_factory);

  // Register the type in factor to use trace logger creatation function.
  trace_logger_factory->register_type(rl_net_native::constants::BINDING_TRACE_LOGGER, binding_tracer_create);

  // This is a clone of cleanup_trace_logger_factory
  std::swap(trace_logger_factory, factory_context->trace_logger_factory);
  if (trace_logger_factory != nullptr && trace_logger_factory != &reinforcement_learning::trace_logger_factory)
  {
    delete trace_logger_factory;
  }

  // Set TRACE_LOG_IMPLEMENTATION configuration to use trace logger.
  config->set(reinforcement_learning::name::TRACE_LOG_IMPLEMENTATION, rl_net_native::constants::BINDING_TRACE_LOGGER);

  context->slates_loop = new reinforcement_learning::slates_loop(*config, pipe_background_error_callback, context,
      factory_context->trace_logger_factory, factory_context->data_transport_factory, factory_context->model_factory,
      factory_context->sender_factory, factory_context->time_provider_factory);

  return context;
}

API void DeleteSlatesLoop(slates_loop_context_t* context)
{
  // Since the slates_loop destructor waits for queues to drain, this can have unhappy consequences,
  // so detach the callback pipe first. This will cause all background callbacks to no-op in the
  // unmanaged side, which maintains expected thread semantics (the user of the bindings)
  context->loop_context.background_error_callback = nullptr;
  context->loop_context.trace_logger_callback = nullptr;

  delete context->loop_context.trace_logger_factory;
  delete context->slates_loop;
  delete context;
}

API int SlatesLoopInit(slates_loop_context_t* context, reinforcement_learning::api_status* status)
{
  return context->slates_loop->init(status);
}

API int SlatesLoopRequestMultiSlotDecision(slates_loop_context_t* context, const char* event_id,
    const char* context_json, int context_json_size, reinforcement_learning::multi_slot_response* resp,
    reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  if (event_id == nullptr)
    return context->slates_loop->request_multi_slot_decision(
        {context_json, static_cast<size_t>(context_json_size)}, *resp, status);
  else
    return context->slates_loop->request_multi_slot_decision(
        event_id, {context_json, static_cast<size_t>(context_json_size)}, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int SlatesLoopRequestMultiSlotDecisionWithFlags(slates_loop_context_t* context, const char* event_id,
    const char* context_json, int context_json_size, unsigned int flags,
    reinforcement_learning::multi_slot_response* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  if (event_id == nullptr)
    return context->slates_loop->request_multi_slot_decision(
        {context_json, static_cast<size_t>(context_json_size)}, flags, *resp, status);
  else
    return context->slates_loop->request_multi_slot_decision(
        event_id, {context_json, static_cast<size_t>(context_json_size)}, flags, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int SlatesLoopRequestMultiSlotDecisionWithBaselineAndFlags(slates_loop_context_t* context, const char* event_id,
    const char* context_json, int context_json_size, unsigned int flags,
    reinforcement_learning::multi_slot_response* resp, const int* baseline_actions, size_t baseline_actions_size,
    reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  return context->slates_loop->request_multi_slot_decision(event_id,
      {context_json, static_cast<size_t>(context_json_size)}, flags, *resp, baseline_actions, baseline_actions_size,
      status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int SlatesLoopRequestMultiSlotDecisionDetailed(slates_loop_context_t* context, const char* event_id,
    const char* context_json, int context_json_size, reinforcement_learning::multi_slot_response_detailed* resp,
    reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  if (event_id == nullptr)
    return context->slates_loop->request_multi_slot_decision(
        {context_json, static_cast<size_t>(context_json_size)}, *resp, status);
  else
    return context->slates_loop->request_multi_slot_decision(
        event_id, {context_json, static_cast<size_t>(context_json_size)}, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int SlatesLoopRequestMultiSlotDecisionDetailedWithFlags(slates_loop_context_t* context, const char* event_id,
    const char* context_json, int context_json_size, unsigned int flags,
    reinforcement_learning::multi_slot_response_detailed* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  if (event_id == nullptr)
    return context->slates_loop->request_multi_slot_decision(
        {context_json, static_cast<size_t>(context_json_size)}, flags, *resp, status);
  else
    return context->slates_loop->request_multi_slot_decision(
        event_id, {context_json, static_cast<size_t>(context_json_size)}, flags, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int SlatesLoopRequestMultiSlotDecisionDetailedWithBaselineAndFlags(slates_loop_context_t* context,
    const char* event_id, const char* context_json, int context_json_size, unsigned int flags,
    reinforcement_learning::multi_slot_response_detailed* resp, const int* baseline_actions,
    size_t baseline_actions_size, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  return context->slates_loop->request_multi_slot_decision(event_id,
      {context_json, static_cast<size_t>(context_json_size)}, flags, *resp, baseline_actions, baseline_actions_size,
      status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int SlatesLoopReportActionTaken(
    slates_loop_context_t* context, const char* event_id, reinforcement_learning::api_status* status)
{
  return context->slates_loop->report_action_taken(event_id, status);
}

API int SlatesLoopReportActionMultiIdTaken(slates_loop_context_t* context, const char* primary_id,
    const char* secondary_id, reinforcement_learning::api_status* status)
{
  return context->slates_loop->report_action_taken(primary_id, secondary_id, status);
}

API int SlatesLoopReportOutcomeF(
    slates_loop_context_t* context, const char* event_id, float outcome, reinforcement_learning::api_status* status)
{
  return context->slates_loop->report_outcome(event_id, outcome, status);
}

API int SlatesLoopReportOutcomeJson(slates_loop_context_t* context, const char* event_id, const char* outcome_json,
    reinforcement_learning::api_status* status)
{
  return context->slates_loop->report_outcome(event_id, outcome_json, status);
}

API int SlatesLoopRefreshModel(slates_loop_context_t* context, reinforcement_learning::api_status* status)
{
  return context->slates_loop->refresh_model(status);
}

API void SlatesLoopSetCallback(slates_loop_context_t* slates_loop, rl_net_native::background_error_callback_t callback)
{
  slates_loop->loop_context.background_error_callback = callback;
}

API void SlatesLoopSetTrace(slates_loop_context_t* slates_loop, rl_net_native::trace_logger_callback_t callback)
{
  slates_loop->loop_context.trace_logger_callback = callback;
}