#include "rl.net.factory_context.h"

#include "error_callback_fn.h"

using namespace reinforcement_learning;

API factory_context_t* CreateFactoryContext()
{
  auto context = new factory_context_t;

  context->data_transport_factory = &data_transport_factory;
  context->model_factory = &model_factory;
  context->sender_factory = &sender_factory;
  context->trace_logger_factory = &trace_logger_factory;
  context->time_provider_factory = &time_provider_factory;

  return context;
}

void cleanup_data_transport_factory(data_transport_factory_t* data_transport_factory)
{
  if (data_transport_factory != nullptr && data_transport_factory != &reinforcement_learning::data_transport_factory)
  {
    // We overrode the built-in data_transport_factory
    delete data_transport_factory;
  }
}

API factory_context_t* CreateFactoryContextWithStaticModel(const char* vw_model, const size_t len)
{
  using namespace reinforcement_learning::model_management;
  auto context = CreateFactoryContext();
  char* vw_model_copy = new char[len];
  std::memcpy(vw_model_copy, vw_model, len);

  auto data_transport_factory_fn = [vw_model_copy, len](std::unique_ptr<model_management::i_data_transport>& retval,
                                       const utility::configuration& configuration, i_trace* trace_logger,
                                       api_status* status) -> int
  {
    retval.reset(new rl_net_native::binding_static_model(vw_model_copy, len));
    return error_code::success;
  };

  data_transport_factory_t* data_transport_factory =
      new data_transport_factory_t(reinforcement_learning::data_transport_factory);
  data_transport_factory->register_type(rl_net_native::constants::BINDING_DATA_TRANSPORT, data_transport_factory_fn);

  std::swap(context->data_transport_factory, data_transport_factory);
  cleanup_data_transport_factory(data_transport_factory);

  return context;
}

void cleanup_trace_logger_factory(trace_logger_factory_t* trace_logger_factory)
{
  if (trace_logger_factory != nullptr && trace_logger_factory != &reinforcement_learning::trace_logger_factory)
  {
    // We overrode the built-in trace_logger_factory
    delete trace_logger_factory;
  }
}

void cleanup_sender_factory(sender_factory_t* sender_factory)
{
  if (sender_factory != nullptr && sender_factory != &reinforcement_learning::sender_factory)
  {
    // We overrode the built-in sender_factory
    delete sender_factory;
  }
}

API void DeleteFactoryContext(factory_context_t* context)
{
  cleanup_trace_logger_factory(context->trace_logger_factory);
  cleanup_sender_factory(context->sender_factory);
  cleanup_data_transport_factory(context->data_transport_factory);

  // TODO: Once we project the others, we will need to add them to cleanup.

  delete context;
}

void invoke_error_callback(error_callback_fn* error_callback, api_status* status)
{
  ERROR_CALLBACK(error_callback, *status);
}

API void SetFactoryContextBindingSenderFactory(
    factory_context_t* context, rl_net_native::sender_create_fn create_fn, rl_net_native::sender_vtable vtable)
{
  auto sender_factory_fn = [=](std::unique_ptr<i_sender>& retval, const utility::configuration& configuration,
                               error_callback_fn* error_callback, i_trace* trace_logger, api_status* status)
  {
    void* managed_handle = create_fn(&configuration, invoke_error_callback, error_callback);
    retval.reset(new rl_net_native::binding_sender(managed_handle, vtable, trace_logger));

    return error_code::success;
  };

  sender_factory_t* sender_factory = new sender_factory_t(reinforcement_learning::sender_factory);
  sender_factory->register_type(rl_net_native::constants::BINDING_SENDER, sender_factory_fn);

  std::swap(context->sender_factory, sender_factory);
  cleanup_sender_factory(sender_factory);
}
