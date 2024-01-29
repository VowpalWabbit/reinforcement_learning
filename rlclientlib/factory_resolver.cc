#include "factory_resolver.h"

#include "constants.h"
#include "err_constants.h"
#include "logger/event_logger.h"
#include "model_mgmt/empty_data_transport.h"
#include "utility/watchdog.h"
#include "vw_model/pdf_model.h"
#include "vw_model/vw_model.h"

#ifdef USE_AZURE_FACTORIES
#  include "azure_factories.h"
#  include "model_mgmt/restapi_data_transport.h"
#endif

#include "console_tracer.h"
#include "error_callback_fn.h"
#include "logger/file/file_logger.h"
#include "model_mgmt/file_model_loader.h"

#include <type_traits>

namespace reinforcement_learning
{
namespace m = model_management;
namespace u = utility;
// For proper static intialization
// Check https://en.wikibooks.org/wiki/More_C++_Idioms/Nifty_Counter for explanation
static int init_guard;  // guaranteed to be zero when loaded

// properly aligned memory for the factory object
template <typename T>
using natural_align = std::aligned_storage<sizeof(T), alignof(T)>;

static natural_align<data_transport_factory_t>::type dtfactory_buf;
static natural_align<model_factory_t>::type modelfactory_buf;
static natural_align<sender_factory_t>::type senderfactory_buf;
static natural_align<trace_logger_factory_t>::type traceloggerfactory_buf;
static natural_align<time_provider_factory_t>::type time_provider_factory_buf;

// Reference should point to the allocated memory to be initialized by placement new in
// factory_initializer::factory_initializer()
data_transport_factory_t& data_transport_factory = (data_transport_factory_t&)(dtfactory_buf);
model_factory_t& model_factory = (model_factory_t&)(modelfactory_buf);
sender_factory_t& sender_factory = (sender_factory_t&)(senderfactory_buf);
trace_logger_factory_t& trace_logger_factory = (trace_logger_factory_t&)(traceloggerfactory_buf);
time_provider_factory_t& time_provider_factory = (time_provider_factory_t&)(time_provider_factory_buf);

factory_initializer::factory_initializer()
{
  if (init_guard++ == 0)
  {
    new (&data_transport_factory) data_transport_factory_t();
    new (&model_factory) model_factory_t();
    new (&sender_factory) sender_factory_t();
    new (&trace_logger_factory) trace_logger_factory_t();
    new (&time_provider_factory) time_provider_factory_t();

    register_default_factories();
  }
}

factory_initializer::~factory_initializer()
{
  if (--init_guard == 0)
  {
    (&data_transport_factory)->~data_transport_factory_t();
    (&model_factory)->~model_factory_t();
    (&sender_factory)->~sender_factory_t();
    (&trace_logger_factory)->~trace_logger_factory_t();
    (&time_provider_factory)->~time_provider_factory_t();
  }
}

void register_default_factories_callback(oauth_callback_t callback)
{
#ifdef USE_AZURE_FACTORIES
  register_azure_oauth_factories(callback);
#endif
}

template <typename model_t>
int model_create(
    std::unique_ptr<m::i_model>& retval, const u::configuration& c, i_trace* trace_logger, api_status* status)
{
  retval.reset(new model_t(trace_logger, c));
  return error_code::success;
}

int null_tracer_create(
    std::unique_ptr<i_trace>& retval, const u::configuration& /*cfg*/, i_trace* trace_logger, api_status* status);
int console_tracer_create(
    std::unique_ptr<i_trace>& retval, const u::configuration& /*cfg*/, i_trace* trace_logger, api_status* status);

int file_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg, const char* file_name,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status)
{
  retval.reset(new logger::file::file_logger(file_name, trace_logger));
  return error_code::success;
}

int empty_data_transport_create(std::unique_ptr<m::i_data_transport>& retval, const u::configuration& config,
    i_trace* trace_logger, api_status* status)
{
  TRACE_INFO(trace_logger, "Empty data transport created.");
  retval.reset(new model_management::empty_data_transport());
  return error_code::success;
}

int file_model_loader_create(std::unique_ptr<m::i_data_transport>& retval, const u::configuration& config,
    i_trace* trace_logger, api_status* status)
{
  TRACE_INFO(trace_logger, "File model loader created.");
  const char* file_name = config.get(name::MODEL_FILE_NAME, "current");
  const bool file_must_exist = config.get_bool(name::MODEL_FILE_MUST_EXIST, false);
  auto file_loader = VW::make_unique<model_management::file_model_loader>(file_name, file_must_exist, trace_logger);

  const auto success = file_loader->init(status);
  if (success != error_code::success) { return success; }

  retval = std::move(file_loader);
  return error_code::success;
}

int null_time_provider_create(
    std::unique_ptr<i_time_provider>& retval, const u::configuration& config, i_trace* trace_logger, api_status* status)
{
  TRACE_INFO(trace_logger, "Null time provider created.");
  retval.reset();
  return error_code::success;
}

int clock_time_provider_create(
    std::unique_ptr<i_time_provider>& retval, const u::configuration& config, i_trace* trace_logger, api_status* status)
{
  TRACE_INFO(trace_logger, "Clock time provider created.");
  retval.reset(new clock_time_provider());
  return error_code::success;
}

void factory_initializer::register_default_factories()
{
#ifdef USE_AZURE_FACTORIES
  register_azure_factories();
#endif

  data_transport_factory.register_type(value::NO_MODEL_DATA, empty_data_transport_create);
  data_transport_factory.register_type(value::FILE_MODEL_DATA, file_model_loader_create);

  model_factory.register_type(value::VW, model_create<m::vw_model>);
  model_factory.register_type(value::PASSTHROUGH_PDF_MODEL, model_create<m::pdf_model>);

  trace_logger_factory.register_type(value::NULL_TRACE_LOGGER, null_tracer_create);
  trace_logger_factory.register_type(value::CONSOLE_TRACE_LOGGER, console_tracer_create);

  time_provider_factory.register_type(value::NULL_TIME_PROVIDER, null_time_provider_create);
  time_provider_factory.register_type(value::CLOCK_TIME_PROVIDER, clock_time_provider_create);

  // Register File loggers
  sender_factory.register_type(value::EPISODE_FILE_SENDER,
      [](std::unique_ptr<i_sender>& retval, const u::configuration& c, error_callback_fn* cb, i_trace* trace_logger,
          api_status* status)
      {
        const char* file_name = c.get(name::EPISODE_FILE_NAME, "episode.fb.data");
        return file_sender_create(retval, c, file_name, cb, trace_logger, status);
      });
  sender_factory.register_type(value::OBSERVATION_FILE_SENDER,
      [](std::unique_ptr<i_sender>& retval, const u::configuration& c, error_callback_fn* cb, i_trace* trace_logger,
          api_status* status)
      {
        const char* file_name = c.get(name::OBSERVATION_FILE_NAME, "observation.fb.data");
        return file_sender_create(retval, c, file_name, cb, trace_logger, status);
      });
  sender_factory.register_type(value::INTERACTION_FILE_SENDER,
      [](std::unique_ptr<i_sender>& retval, const u::configuration& c, error_callback_fn* cb, i_trace* trace_logger,
          api_status* status)
      {
        const char* file_name = c.get(name::INTERACTION_FILE_NAME, "interaction.fb.data");
        return file_sender_create(retval, c, file_name, cb, trace_logger, status);
      });
}

int null_tracer_create(
    std::unique_ptr<i_trace>& retval, const u::configuration& cfg, i_trace* trace_logger, api_status* status)
{
  retval.reset();
  return error_code::success;
}

int console_tracer_create(
    std::unique_ptr<i_trace>& retval, const u::configuration& cfg, i_trace* trace_logger, api_status* status)
{
  retval.reset(new console_tracer());
  return error_code::success;
}
}  // namespace reinforcement_learning
