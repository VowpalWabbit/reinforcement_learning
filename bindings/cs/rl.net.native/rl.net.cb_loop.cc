#include "rl.net.cb_loop.h"

#include "binding_tracer.h"
#include "constants.h"
#include "err_constants.h"
#include "trace_logger.h"

#include <iostream>

static void pipe_background_error_callback(const reinforcement_learning::api_status& status, cb_loop_context_t* context)
{
  auto managed_backgroud_error_callback_local = context->base_loop_context.background_error_callback;
  if (managed_backgroud_error_callback_local) { managed_backgroud_error_callback_local(status); }
}

API cb_loop_context_t* CreateCBLoop(
    reinforcement_learning::utility::configuration* config, factory_context_t* factory_context)
{
  cb_loop_context_t* context = new cb_loop_context_t;
  context->base_loop_context.background_error_callback = nullptr;
  context->base_loop_context.trace_logger_callback = nullptr;
  context->base_loop_context.trace_logger_factory = nullptr;

  // Create a trace log factory by passing in below creator. It allows CBLoop to use trace_logger provided by user.
  const auto binding_tracer_create = [context](std::unique_ptr<reinforcement_learning::i_trace>& retval,
                                         const reinforcement_learning::utility::configuration& cfg,
                                         reinforcement_learning::i_trace* trace_logger,
                                         reinforcement_learning::api_status* status)
  {
    retval.reset(new rl_net_native::binding_tracer(context->base_loop_context));
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

  context->cb_loop = new reinforcement_learning::cb_loop(*config, pipe_background_error_callback, context,
      factory_context->trace_logger_factory, factory_context->data_transport_factory, factory_context->model_factory,
      factory_context->sender_factory, factory_context->time_provider_factory);

  return context;
}

API void DeleteCBLoop(cb_loop_context_t* context)
{
  // Since the cb_loop destructor waits for queues to drain, this can have unhappy consequences,
  // so detach the callback pipe first. This will cause all background callbacks to no-op in the
  // unmanaged side, which maintains expected thread semantics (the user of the bindings)
  context->base_loop_context.background_error_callback = nullptr;
  context->base_loop_context.trace_logger_callback = nullptr;

  delete context->base_loop_context.trace_logger_factory;
  delete context->cb_loop;
  delete context;
}

API int CBLoopInit(cb_loop_context_t* context, reinforcement_learning::api_status* status)
{
  return context->cb_loop->init(status);
}

API int CBLoopChooseRank(cb_loop_context_t* context, const char* event_id, const char* context_json,
    int context_json_size, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status)
{
  if (event_id == nullptr)
  {
    return context->cb_loop->choose_rank({context_json, static_cast<size_t>(context_json_size)}, *resp, status);
  }

  return context->cb_loop->choose_rank(event_id, {context_json, static_cast<size_t>(context_json_size)}, *resp, status);
}

API int CBLoopChooseRankWithFlags(cb_loop_context_t* context, const char* event_id, const char* context_json,
    int context_json_size, unsigned int flags, reinforcement_learning::ranking_response* resp,
    reinforcement_learning::api_status* status)
{
  return context->cb_loop->choose_rank(
      event_id, {context_json, static_cast<size_t>(context_json_size)}, flags, *resp, status);
}

API int CBLoopReportActionTaken(
    cb_loop_context_t* context, const char* event_id, reinforcement_learning::api_status* status)
{
  return context->cb_loop->report_action_taken(event_id, status);
}

API int CBLoopReportActionMultiIdTaken(cb_loop_context_t* context, const char* primary_id, const char* secondary_id,
    reinforcement_learning::api_status* status)
{
  return context->cb_loop->report_action_taken(primary_id, secondary_id, status);
}

API int CBLoopReportOutcomeF(
    cb_loop_context_t* context, const char* event_id, float outcome, reinforcement_learning::api_status* status)
{
  return context->cb_loop->report_outcome(event_id, outcome, status);
}

API int CBLoopReportOutcomeJson(cb_loop_context_t* context, const char* event_id, const char* outcome_json,
    reinforcement_learning::api_status* status)
{
  return context->cb_loop->report_outcome(event_id, outcome_json, status);
}

API int CBLoopRefreshModel(cb_loop_context_t* context, reinforcement_learning::api_status* status)
{
  return context->cb_loop->refresh_model(status);
}

API void CBLoopSetCallback(cb_loop_context_t* cb_loop, rl_net_native::background_error_callback_t callback)
{
  cb_loop->base_loop_context.background_error_callback = callback;
}

API void CBLoopSetTrace(cb_loop_context_t* cb_loop, rl_net_native::trace_logger_callback_t callback)
{
  cb_loop->base_loop_context.trace_logger_callback = callback;
}
