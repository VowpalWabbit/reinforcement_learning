#include "rl.net.cb_logger.h"
#include "binding_tracer.h"
#include "constants.h"
#include "err_constants.h"

#include "trace_logger.h"


static void pipe_background_error_callback(const reinforcement_learning::api_status& status, cb_logger_context_t* context)
{
  auto managed_backgroud_error_callback_local = context->background_error_callback;
  if (managed_backgroud_error_callback_local)
  {
    managed_backgroud_error_callback_local(status);
  }
}

API cb_logger_context_t* CreateCbLogger(reinforcement_learning::utility::configuration* config)
{
  cb_logger_context_t* context = new cb_logger_context_t;
  context->background_error_callback = nullptr;
  context->trace_logger_callback = nullptr;
  context->trace_logger_factory = nullptr;

  // Create a trace log factory by passing in below creator. It allows LiveModel to use trace_logger provided by user.
  const auto binding_tracer_create = [context](reinforcement_learning::i_trace** retval,
    const reinforcement_learning::utility::configuration& cfg,
    reinforcement_learning::i_trace* trace_logger,
    reinforcement_learning::api_status* status)
  {
    *retval = new rl_net_native::binding_tracer<cb_logger_context_t>(*context);
    return reinforcement_learning::error_code::success;
  };

  context->trace_logger_factory = new reinforcement_learning::trace_logger_factory_t();;

  // Register the type in factor to use trace logger creatation function.
  context->trace_logger_factory->register_type(rl_net_native::constants::BINDING_TRACE_LOGGER, binding_tracer_create);

  // Set TRACE_LOG_IMPLEMENTATION configuration to use trace logger.
  config->set(reinforcement_learning::name::TRACE_LOG_IMPLEMENTATION, rl_net_native::constants::BINDING_TRACE_LOGGER);

  context->logger = new reinforcement_learning::cb::logger(*config, pipe_background_error_callback, context, context->trace_logger_factory);

  return context;
}

API void DeleteCbLogger(cb_logger_context_t* context)
{
  // Since the livemodel destructor waits for queues to drain, this can have unhappy consequences,
  // so detach the callback pipe first. This will cause all background callbacks to no-op in the
  // unmanaged side, which maintains expected thread semantics (the user of the bindings)
  context->background_error_callback = nullptr;
  context->trace_logger_callback = nullptr;

  delete context->trace_logger_factory;
  delete context->logger;
  delete context;
}

API int CbLoggerInit(cb_logger_context_t* context, reinforcement_learning::api_status* status)
{
  return context->logger->init(status);
}

API int CbLoggerLogF(cb_logger_context_t* context, const char * event_id, const char * context_json, const reinforcement_learning::ranking_response* resp, float outcome, reinforcement_learning::api_status* status)
{
  if (event_id == nullptr)
  {
    return context->logger->log(context_json, *resp, outcome, status);
  }

  return context->logger->log(event_id, context_json, *resp, outcome, status);
}

API int CbLoggerLogJson(cb_logger_context_t* context, const char * event_id, const char * context_json, const  reinforcement_learning::ranking_response* resp, const char* outcome, reinforcement_learning::api_status* status)
{
  if (event_id == nullptr)
  {
    return context->logger->log(context_json, *resp, outcome, status);
  }

  return context->logger->log(event_id, context_json, *resp, outcome, status);
}

API void CbLoggerSetCallback(cb_logger_context_t* context, rl_net_native::background_error_callback_t callback)
{
  context->background_error_callback = callback;
}

API void CbLoggerSetTrace(cb_logger_context_t* context, rl_net_native::trace_logger_callback_t callback)
{
  context->trace_logger_callback = callback;
}
