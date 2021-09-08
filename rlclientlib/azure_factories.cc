#include "azure_factories.h"

#include "factory_resolver.h"
#include "constants.h"
#include "model_mgmt/restapi_data_transport.h"
#include "logger/event_logger.h"
#include "logger/http_transport_client.h"
#include "utility/http_helper.h"
#include "utility/authorization.h"
#include "utility/apim_http_authorization.h"
#include "utility/eventhub_http_authorization.h"

namespace reinforcement_learning {
  namespace m = model_management;
  namespace u = utility;

  int restapi_data_transport_create(m::i_data_transport** retval, const u::configuration& config, i_trace* trace_logger, api_status* status);
  int observation_sender_create(i_sender** retval, const u::configuration&, error_callback_fn*, i_trace* trace_logger, api_status* status);
  int interaction_sender_create(i_sender** retval, const u::configuration&, error_callback_fn*, i_trace* trace_logger, api_status* status);
  int decision_sender_create(i_sender** retval, const u::configuration&, error_callback_fn*, i_trace* trace_logger, api_status* status);
  int observation_api_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status);
  int interaction_api_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status);
 
  void register_azure_factories() {
    data_transport_factory.register_type(value::AZURE_STORAGE_BLOB, restapi_data_transport_create);
    sender_factory.register_type(value::OBSERVATION_EH_SENDER, observation_sender_create);
    sender_factory.register_type(value::INTERACTION_EH_SENDER, interaction_sender_create);
    sender_factory.register_type(value::OBSERVATION_API_SENDER, observation_api_sender_create);
    sender_factory.register_type(value::INTERACTION_API_SENDER, interaction_api_sender_create);
  }

  int restapi_data_transport_create(m::i_data_transport** retval, const u::configuration& config, i_trace* trace_logger, api_status* status) {
    const auto uri = config.get(name::MODEL_BLOB_URI, nullptr);
    if (uri == nullptr) {
      RETURN_ERROR(trace_logger, status, http_uri_not_provided);
    }
    i_http_client* client;
    RETURN_IF_FAIL(create_http_client(uri, config, &client, status));
    *retval = new m::restapi_data_transport(client, trace_logger);
    return error_code::success;
  }

  std::string build_eh_url(const char* eh_host, const char* eh_name) {
    std::string url;
    url.append("https://").append(eh_host).append("/").append(eh_name)
      .append("/messages?timeout=60&api-version=2014-01");
    return url;
  }

  // Creates i_sender object for sending observations data to the apim endpoint.
  int observation_api_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status) {
    const auto api_host = cfg.get(name::OBSERVATION_API_HOST, "localhost:8080");
    const auto api_key = cfg.get(name::API_KEY, "dummykey");

    std::string url;
    url.append(api_host);

    i_http_client* client;
    RETURN_IF_FAIL(create_http_client(url.c_str(), cfg, &client, status));

    i_authorization* authorization = new apim_http_authorization(api_key);
    *retval = new http_transport_client(
      client,
      cfg.get_int(name::OBSERVATION_APIM_TASKS_LIMIT, 16),
      cfg.get_int(name::OBSERVATION_APIM_MAX_HTTP_RETRIES, 4),
      trace_logger,
      error_cb,
      authorization);

    return error_code::success;
  }

  // Creates i_sender object for sending interactions data to the apim endpoint.
  int interaction_api_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status) {	  
    const auto api_host = cfg.get(name::INTERACTION_API_HOST, "localhost:8080");
    const auto api_key = cfg.get(name::API_KEY, "dummykey");

    std::string url;
    url.append(api_host);

    i_http_client* client;
    RETURN_IF_FAIL(create_http_client(url.c_str(), cfg, &client, status));

    i_authorization* authorization = new apim_http_authorization(api_key);
    *retval = new http_transport_client(
      client,
      cfg.get_int(name::INTERACTION_APIM_TASKS_LIMIT, 16),
      cfg.get_int(name::INTERACTION_APIM_MAX_HTTP_RETRIES, 4),
      trace_logger,
      error_cb,
      authorization);

    return error_code::success;
  }

  // Creates i_sender object for sending observations data to the event hub.
  int observation_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status) {
    const auto eh_host = cfg.get(name::OBSERVATION_EH_HOST, "localhost:8080");
    const auto eh_name = cfg.get(name::OBSERVATION_EH_NAME, "observation");
    const auto eh_url = build_eh_url(eh_host, eh_name);

    i_http_client* client;
    RETURN_IF_FAIL(create_http_client(eh_url.c_str(), cfg, &client, status));

    i_authorization* authorization = new eventhub_http_authorization(
      eh_host,
      cfg.get(name::OBSERVATION_EH_KEY_NAME, ""),
      cfg.get(name::OBSERVATION_EH_KEY, ""),
      eh_name,
      trace_logger);
    *retval = new http_transport_client(
      client,
      cfg.get_int(name::OBSERVATION_EH_TASKS_LIMIT, 16),
      cfg.get_int(name::OBSERVATION_EH_MAX_HTTP_RETRIES, 4),
      trace_logger,
      error_cb,
      authorization);

    return error_code::success;
  }

  // Creates i_sender object for sending interactions data to the event hub.
  int interaction_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status) {
    const auto eh_host = cfg.get(name::INTERACTION_EH_HOST, "localhost:8080");
    const auto eh_name = cfg.get(name::INTERACTION_EH_NAME, "interaction");
    const auto eh_url = build_eh_url(eh_host, eh_name);

    i_http_client* client;
    RETURN_IF_FAIL(create_http_client(eh_url.c_str(), cfg, &client, status));

    i_authorization* authorization = new eventhub_http_authorization(
      eh_host,
      cfg.get(name::INTERACTION_EH_KEY_NAME, ""),
      cfg.get(name::INTERACTION_EH_KEY, ""),
      eh_name,
      trace_logger);
    *retval = new http_transport_client(
      client,
      cfg.get_int(name::INTERACTION_EH_TASKS_LIMIT, 16),
      cfg.get_int(name::INTERACTION_EH_MAX_HTTP_RETRIES, 4),
      trace_logger,
      error_cb,
      authorization);

    return error_code::success;
  }
}
