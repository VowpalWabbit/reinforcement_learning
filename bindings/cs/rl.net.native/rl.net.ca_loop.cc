#include "rl.net.ca_loop.h"

#include "binding_tracer.h"
#include "constants.h"
#include "err_constants.h"
#include "trace_logger.h"

#include <iostream>

static void pipe_background_error_callback(const reinforcement_learning::api_status& status, ca_loop_context_t* context)
{
  auto managed_backgroud_error_callback_local = context->loop_context.background_error_callback;
  if (managed_backgroud_error_callback_local) { managed_backgroud_error_callback_local(status); }
}

API ca_loop_context_t* CreateCALoop(
    reinforcement_learning::utility::configuration* config, factory_context_t* factory_context)
{
  ca_loop_context_t* context = new ca_loop_context_t;
  context->loop_context.background_error_callback = nullptr;
  context->loop_context.trace_logger_callback = nullptr;
  context->loop_context.trace_logger_factory = nullptr;

  // Create a trace log factory by passing in below creator. It allows CALoop to use trace_logger provided by user.
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

  context->ca_loop = new reinforcement_learning::ca_loop(*config, pipe_background_error_callback, context,
      factory_context->trace_logger_factory, factory_context->data_transport_factory, factory_context->model_factory,
      factory_context->sender_factory, factory_context->time_provider_factory);

  return context;
}

API void DeleteCALoop(ca_loop_context_t* context)
{
  // Since the ca_loop destructor waits for queues to drain, this can have unhappy consequences,
  // so detach the callback pipe first. This will cause all background callbacks to no-op in the
  // unmanaged side, which maintains expected thread semantics (the user of the bindings)
  context->loop_context.background_error_callback = nullptr;
  context->loop_context.trace_logger_callback = nullptr;

  delete context->loop_context.trace_logger_factory;
  delete context->ca_loop;
  delete context;
}

API int CALoopInit(ca_loop_context_t* context, reinforcement_learning::api_status* status)
{
  return context->ca_loop->init(status);
}

API int CALoopRequestContinuousAction(ca_loop_context_t* context, const char* event_id, const char* context_json,
    int context_json_size, reinforcement_learning::continuous_action_response* resp,
    reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  return context->ca_loop->request_continuous_action(
      event_id, {context_json, static_cast<size_t>(context_json_size)}, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int CALoopRequestContinuousActionWithFlags(ca_loop_context_t* context, const char* event_id,
    const char* context_json, int context_json_size, unsigned int flags,
    reinforcement_learning::continuous_action_response* resp, reinforcement_learning::api_status* status)
{
  RL_IGNORE_DEPRECATED_USAGE_START
  return context->ca_loop->request_continuous_action(
      {context_json, static_cast<size_t>(context_json_size)}, flags, *resp, status);
  RL_IGNORE_DEPRECATED_USAGE_END
}

API int CALoopReportActionTaken(
    ca_loop_context_t* context, const char* event_id, reinforcement_learning::api_status* status)
{
  return context->ca_loop->report_action_taken(event_id, status);
}

API int CALoopReportActionMultiIdTaken(ca_loop_context_t* context, const char* primary_id, const char* secondary_id,
    reinforcement_learning::api_status* status)
{
  return context->ca_loop->report_action_taken(primary_id, secondary_id, status);
}

API int CALoopReportOutcomeF(
    ca_loop_context_t* context, const char* event_id, float outcome, reinforcement_learning::api_status* status)
{
  return context->ca_loop->report_outcome(event_id, outcome, status);
}

API int CALoopReportOutcomeJson(ca_loop_context_t* context, const char* event_id, const char* outcome_json,
    reinforcement_learning::api_status* status)
{
  return context->ca_loop->report_outcome(event_id, outcome_json, status);
}

API int CALoopRefreshModel(ca_loop_context_t* context, reinforcement_learning::api_status* status)
{
  return context->ca_loop->refresh_model(status);
}

API void CALoopSetCallback(ca_loop_context_t* ca_loop, rl_net_native::background_error_callback_t callback)
{
  ca_loop->loop_context.background_error_callback = callback;
}

API void CALoopSetTrace(ca_loop_context_t* ca_loop, rl_net_native::trace_logger_callback_t callback)
{
  ca_loop->loop_context.trace_logger_callback = callback;
}