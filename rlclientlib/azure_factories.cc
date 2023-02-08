#include "azure_factories.h"

#include "constants.h"
#include "factory_resolver.h"
#include "logger/event_logger.h"
#include "logger/http_transport_client.h"
#include "model_mgmt/restapi_data_transport.h"
#include "utility/eventhub_http_authorization.h"
#include "utility/header_authorization.h"
#include "utility/http_helper.h"

namespace reinforcement_learning
{
namespace m = model_management;
namespace u = utility;

int restapi_data_transport_create(std::unique_ptr<m::i_data_transport>& retval, const u::configuration& config,
    i_trace* trace_logger, api_status* status);
int authenticated_restapi_data_transport_create(std::unique_ptr<m::i_data_transport>& retval,
    const u::configuration& config, i_trace* trace_logger, api_status* status);
int episode_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& /*cfg*/,
    error_callback_fn* /*error_cb*/, i_trace* trace_logger, api_status* status);
int observation_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& /*cfg*/,
    error_callback_fn* /*error_cb*/, i_trace* trace_logger, api_status* status);
int interaction_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& /*cfg*/,
    error_callback_fn* /*error_cb*/, i_trace* trace_logger, api_status* status);
int decision_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration&, error_callback_fn*,
    i_trace* trace_logger, api_status* status);
int episode_api_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status);
int observation_api_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status);
int interaction_api_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status);

void register_azure_factories()
{
  data_transport_factory.register_type(value::AZURE_STORAGE_BLOB, restapi_data_transport_create);
  data_transport_factory.register_type(value::HTTP_MODEL_DATA, authenticated_restapi_data_transport_create);
  sender_factory.register_type(value::OBSERVATION_EH_SENDER, observation_sender_create);
  sender_factory.register_type(value::INTERACTION_EH_SENDER, interaction_sender_create);
  sender_factory.register_type(value::EPISODE_EH_SENDER, episode_sender_create);
  sender_factory.register_type(value::OBSERVATION_HTTP_API_SENDER, observation_api_sender_create);
  sender_factory.register_type(value::INTERACTION_HTTP_API_SENDER, interaction_api_sender_create);
  sender_factory.register_type(value::EPISODE_HTTP_API_SENDER, episode_api_sender_create);
}

int restapi_data_transport_create(std::unique_ptr<m::i_data_transport>& retval, const u::configuration& config,
    i_trace* trace_logger, api_status* status)
{
  const auto* const uri = config.get(name::MODEL_BLOB_URI, nullptr);
  if (uri == nullptr) { RETURN_ERROR(trace_logger, status, http_model_uri_not_provided); }
  i_http_client* client = nullptr;
  RETURN_IF_FAIL(create_http_client(uri, config, &client, status));
  retval.reset(new m::restapi_data_transport(client, trace_logger));
  return error_code::success;
}

int authenticated_restapi_data_transport_create(std::unique_ptr<m::i_data_transport>& retval,
    const u::configuration& config, i_trace* trace_logger, api_status* status)
{
  const auto* model_uri = config.get(name::MODEL_BLOB_URI, nullptr);
  if (model_uri == nullptr) { RETURN_ERROR(trace_logger, status, http_model_uri_not_provided); }
  i_http_client* client = nullptr;
  RETURN_IF_FAIL(create_http_client(model_uri, config, &client, status));
  retval.reset(new m::restapi_data_transport(
      std::unique_ptr<i_http_client>(client), config, m::model_source::HTTP_API, trace_logger));
  return error_code::success;
}

std::string build_eh_url(const char* eh_host, const char* eh_name)
{
  std::string url;
  url.append("https://").append(eh_host).append("/").append(eh_name).append("/messages?timeout=60&api-version=2014-01");
  return url;
}

int episode_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg, error_callback_fn* error_cb,
    i_trace* trace_logger, api_status* status)
{
  const auto* const eh_host = cfg.get(name::EPISODE_EH_HOST, "localhost:8080");
  const auto* const eh_name = cfg.get(name::EPISODE_EH_NAME, "episode");
  const auto eh_url = build_eh_url(eh_host, eh_name);
  i_http_client* client = nullptr;
  RETURN_IF_FAIL(create_http_client(eh_url.c_str(), cfg, &client, status));
  retval.reset(new http_transport_client<eventhub_http_authorization>(client,
      cfg.get_int(name::EPISODE_EH_TASKS_LIMIT, 16), cfg.get_int(name::EPISODE_EH_MAX_HTTP_RETRIES, 4),
      std::chrono::milliseconds(cfg.get_int(name::EPISODE_EH_MAX_HTTP_RETRY_DURATION_MS, 3600000)), trace_logger,
      error_cb));
  return error_code::success;
}

int create_apim_http_api_sender(std::unique_ptr<i_sender>& retval, const u::configuration& cfg, const char* api_host,
    int tasks_limit, int max_http_retries, std::chrono::milliseconds max_http_retry_duration,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status)
{
  i_http_client* client = nullptr;
  RETURN_IF_FAIL(create_http_client(api_host, cfg, &client, status));
  retval.reset(new http_transport_client<header_authorization>(
      client, tasks_limit, max_http_retries, max_http_retry_duration, trace_logger, error_cb));
  return error_code::success;
}

// Creates i_sender object for sending episode data to the apim endpoint.
int episode_api_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status)
{
  const auto* const api_host = cfg.get(name::EPISODE_HTTP_API_HOST, "localhost:8080");
  return create_apim_http_api_sender(retval, cfg, api_host, cfg.get_int(name::EPISODE_APIM_MAX_HTTP_RETRIES, 4),
      cfg.get_int(name::EPISODE_APIM_TASKS_LIMIT, 4),
      std::chrono::milliseconds(cfg.get_int(name::EPISODE_APIM_MAX_HTTP_RETRY_DURATION_MS, 3600000)), error_cb,
      trace_logger, status);
}

// Creates i_sender object for sending observations data to the apim endpoint.
int observation_api_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status)
{
  const auto* const api_host = cfg.get(name::OBSERVATION_HTTP_API_HOST, "localhost:8080");
  return create_apim_http_api_sender(retval, cfg, api_host, cfg.get_int(name::OBSERVATION_APIM_TASKS_LIMIT, 16),
      cfg.get_int(name::OBSERVATION_APIM_MAX_HTTP_RETRIES, 4),
      std::chrono::milliseconds(cfg.get_int(name::OBSERVATION_APIM_MAX_HTTP_RETRY_DURATION_MS, 3600000)), error_cb,
      trace_logger, status);
}

// Creates i_sender object for sending interactions data to the apim endpoint.
int interaction_api_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status)
{
  const auto* const api_host = cfg.get(name::INTERACTION_HTTP_API_HOST, "localhost:8080");
  return create_apim_http_api_sender(retval, cfg, api_host, cfg.get_int(name::INTERACTION_APIM_TASKS_LIMIT, 16),
      cfg.get_int(name::INTERACTION_APIM_MAX_HTTP_RETRIES, 4),
      std::chrono::milliseconds(cfg.get_int(name::INTERACTION_APIM_MAX_HTTP_RETRY_DURATION_MS, 3600000)), error_cb,
      trace_logger, status);
}

// Creates i_sender object for sending observations data to the event hub.
int observation_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status)
{
  const auto* const eh_host = cfg.get(name::OBSERVATION_EH_HOST, "localhost:8080");
  const auto* const eh_name = cfg.get(name::OBSERVATION_EH_NAME, "observation");
  const auto eh_url = build_eh_url(eh_host, eh_name);
  i_http_client* client = nullptr;
  RETURN_IF_FAIL(create_http_client(eh_url.c_str(), cfg, &client, status));
  retval.reset(new http_transport_client<eventhub_http_authorization>(client,
      cfg.get_int(name::OBSERVATION_EH_TASKS_LIMIT, 16), cfg.get_int(name::OBSERVATION_EH_MAX_HTTP_RETRIES, 4),
      std::chrono::milliseconds(cfg.get_int(name::OBSERVATION_EH_MAX_HTTP_RETRY_DURATION_MS, 3600000)), trace_logger,
      error_cb));
  return error_code::success;
}

// Creates i_sender object for sending interactions data to the event hub.
int interaction_sender_create(std::unique_ptr<i_sender>& retval, const u::configuration& cfg,
    error_callback_fn* error_cb, i_trace* trace_logger, api_status* status)
{
  const auto* const eh_host = cfg.get(name::INTERACTION_EH_HOST, "localhost:8080");
  const auto* const eh_name = cfg.get(name::INTERACTION_EH_NAME, "interaction");
  const auto eh_url = build_eh_url(eh_host, eh_name);
  i_http_client* client = nullptr;
  RETURN_IF_FAIL(create_http_client(eh_url.c_str(), cfg, &client, status));
  retval.reset(new http_transport_client<eventhub_http_authorization>(client,
      cfg.get_int(name::INTERACTION_EH_TASKS_LIMIT, 16), cfg.get_int(name::INTERACTION_EH_MAX_HTTP_RETRIES, 4),
      std::chrono::milliseconds(cfg.get_int(name::INTERACTION_EH_MAX_HTTP_RETRY_DURATION_MS, 3600000)), trace_logger,
      error_cb));
  return error_code::success;
}
}  // namespace reinforcement_learning
