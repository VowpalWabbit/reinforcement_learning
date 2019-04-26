#include "rl.net.live_model.h"
#include "binding_tracer.h"
#include "constants.h"
#include "err_constants.h"
#include "rl.net.live_model.h"
#include "trace_logger.h"


static void pipe_background_error_callback(const reinforcement_learning::api_status& status, livemodel_context_t* context)
{
    auto managed_backgroud_error_callback_local = context->background_error_callback;
    if (managed_backgroud_error_callback_local)
    {
        managed_backgroud_error_callback_local(status);
    }
}

API livemodel_context_t* CreateLiveModel(reinforcement_learning::utility::configuration* config)
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

    context->trace_logger_factory = new reinforcement_learning::trace_logger_factory_t();;

    // Register the type in factor to use trace logger creatation function.
    context->trace_logger_factory->register_type(rl_net_native::constants::BINDING_TRACE_LOGGER, binding_tracer_create);

    // Set TRACE_LOG_IMPLEMENTATION configuration to use trace logger.
    config->set(reinforcement_learning::name::TRACE_LOG_IMPLEMENTATION, rl_net_native::constants::BINDING_TRACE_LOGGER);

    context->livemodel = new reinforcement_learning::live_model(*config, pipe_background_error_callback, context, context->trace_logger_factory);

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

API int LiveModelChooseRank(livemodel_context_t* context, const char * event_id, const char * context_json, reinforcement_learning::ranking_response* resp, reinforcement_learning::api_status* status)
{
    if (event_id == nullptr)
    {
        return context->livemodel->choose_rank(context_json, *resp, status);
    }

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
