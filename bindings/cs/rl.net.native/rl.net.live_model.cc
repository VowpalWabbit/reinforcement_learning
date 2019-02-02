#include "binding_tracer.h"
#include "constants.h"
#include "err_constants.h"
#include "rl.net.live_model.h"
#include "trace_logger.h"

static void pipe_managed_callback(const reinforcement_learning::api_status& status, livemodel_context_t* context)
{
  auto managed_callback_local = context->callback;
  if (managed_callback_local)
  {
    managed_callback_local(status);
  }
}

API livemodel_context_t* CreateLiveModel(reinforcement_learning::utility::configuration* config)
{
  livemodel_context_t* context = new livemodel_context_t;
  context->callback = nullptr;
  context->trace_logger_callback = nullptr;
  context->trace_logger_factory = nullptr;

  const auto binding_tracer_create = [context](reinforcement_learning::i_trace** retval,
    const reinforcement_learning::utility::configuration& cfg,
    reinforcement_learning::i_trace* trace_logger,
    reinforcement_learning::api_status* status)
  {
    *retval = new reinforcement_learning::binding_tracer(context->trace_logger_callback);
    return reinforcement_learning::error_code::success;
  };

  config->set(reinforcement_learning::name::TRACE_LOG_IMPLEMENTATION, reinforcement_learning::value::BINDING_TRACE_LOGGER);
  context->trace_logger_factory = new reinforcement_learning::trace_logger_factory_t();;
  context->trace_logger_factory->register_type(reinforcement_learning::value::BINDING_TRACE_LOGGER, binding_tracer_create);

  context->livemodel = new reinforcement_learning::live_model(*config, pipe_managed_callback, context, context->trace_logger_factory);

  return context;
}

API void DeleteLiveModel(livemodel_context_t* context)
{
  // Since the livemodel destructor waits for queues to drain, this can have unhappy consequences,
  // so detach the callback pipe first. This will cause all background callbacks to no-op in the
  // unmanaged side, which maintains expected thread semantics (the user of the bindings)
  context->callback = nullptr;
  context->trace_logger_callback = nullptr;

  delete context->trace_logger_factory;
  delete context->livemodel;
  delete context;
}


API int LiveModelInit(livemodel_context_t* context, reinforcement_learning::api_status* status)
{
  return context->livemodel->init(status);
}

API int LiveModelChooseRank(livemodel_context_t* context, const char * event_id, const char * context_json, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status)
{
  return context->livemodel->choose_rank(event_id, context_json, *resp, status);
}

API int LiveModelChooseRankWithFlags(livemodel_context_t* context, const char * event_id, const char * context_json, unsigned int flags, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status)
{
  return context->livemodel->choose_rank(event_id, context_json, flags, *resp, status);
}

API int LiveModelReportActionTaken(livemodel_context_t* context, const char * event_id, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_action_taken(event_id, status);
}

API int LiveModelReportOutcomeF(livemodel_context_t* context, const char * event_id, float outcome, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_outcome(event_id, outcome, status);
}

API int LiveModelReportOutcomeJson(livemodel_context_t* context, const char * event_id, const char * outcomeJson, reinforcement_learning::api_status* status)
{
  return context->livemodel->report_outcome(event_id, outcomeJson, status);
}

API void LiveModelSetCallback(livemodel_context_t* livemodel, managed_callback_t callback)
{
  livemodel->callback = callback;
}

API void LiveModelSetTrace(livemodel_context_t* livemodel, trace_logger_t trace_logger_callback)
{
  livemodel->trace_logger_callback = trace_logger_callback;
}