#include "factory_resolver.h"

#include "constants.h"
#include "err_constants.h"
#include "model_mgmt/restapi_data_transport.h"
#include "vw_model/vw_model.h"
#include "logger/event_logger.h"
#include "utility/watchdog.h"
#include "azure_factories.h"

#include <type_traits>
#include "console_tracer.h"

namespace reinforcement_learning {
  namespace m = model_management;
  namespace u = utility;
  // For proper static intialization
  // Check https://en.wikibooks.org/wiki/More_C++_Idioms/Nifty_Counter for explanation
  static int init_guard;  // guaranteed to be zero when loaded

  // properly aligned memory for the factory object
  template <typename T>
  using natural_align = std::aligned_storage<sizeof(T), alignof ( T )>;

  static natural_align<data_transport_factory_t>::type dtfactory_buf;
  static natural_align<model_factory_t>::type modelfactory_buf;
  static natural_align<sender_factory_t>::type senderfactory_buf;
  static natural_align<trace_logger_factory_t>::type traceloggerfactory_buf;

  // Reference should point to the allocated memory to be initialized by placement new in factory_initializer::factory_initializer()
  data_transport_factory_t& data_transport_factory = (data_transport_factory_t&)( dtfactory_buf );
  model_factory_t& model_factory = (model_factory_t&)( modelfactory_buf );
  sender_factory_t& sender_factory = (sender_factory_t&)( senderfactory_buf );
  trace_logger_factory_t& trace_logger_factory = (trace_logger_factory_t&)( traceloggerfactory_buf );

  factory_initializer::factory_initializer() {
    if ( init_guard++ == 0 ) {
      new ( &data_transport_factory ) data_transport_factory_t();
      new ( &model_factory ) model_factory_t();
      new ( &sender_factory ) sender_factory_t();
      new ( &trace_logger_factory ) trace_logger_factory_t();

      register_default_factories();
    }
  }

  factory_initializer::~factory_initializer() {
    if ( --init_guard == 0 ) {
      ( &data_transport_factory )->~data_transport_factory_t();
      ( &model_factory )->~model_factory_t();
      ( &sender_factory )->~sender_factory_t();
      ( &trace_logger_factory )->~trace_logger_factory_t();
    }
  }

  int vw_model_create(m::i_model** retval, const u::configuration&, i_trace* trace_logger, api_status* status);
  int null_tracer_create(i_trace** retval, const u::configuration&, i_trace* trace_logger, api_status* status);
  int console_tracer_create(i_trace** retval, const u::configuration&, i_trace* trace_logger, api_status* status);
  int binding_tracer_create(i_trace** retval, const u::configuration& cfg, i_trace* trace_logger, api_status* status);

  void factory_initializer::register_default_factories() {
    register_azure_factories();
    model_factory.register_type(value::VW, vw_model_create);
    trace_logger_factory.register_type(value::NULL_TRACE_LOGGER, null_tracer_create);
    trace_logger_factory.register_type(value::CONSOLE_TRACE_LOGGER, console_tracer_create);
    trace_logger_factory.register_type(value::BINDING_TRACE_LOGGER, binding_tracer_create);
  }

  int vw_model_create(m::i_model** retval, const u::configuration&, i_trace* trace_logger, api_status* status) {
    *retval = new m::vw_model(trace_logger);
    return error_code::success;
  }

  int null_tracer_create(i_trace** retval, const u::configuration& cfg, i_trace* trace_logger, api_status* status) {
    *retval = nullptr;
    return error_code::success;
  }

  int console_tracer_create(i_trace** retval, const u::configuration& cfg, i_trace* trace_logger, api_status* status) {
    *retval = new console_tracer();
    return error_code::success;
  }

  int binding_tracer_create(i_trace** retval, const u::configuration& cfg, i_trace* trace_logger, api_status* status) {
    *retval = trace_logger;
    return error_code::success;
  }
}
