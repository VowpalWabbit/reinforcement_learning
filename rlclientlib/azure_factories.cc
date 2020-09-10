#include "azure_factories.h"

#include "factory_resolver.h"
#include "constants.h"
#include "model_mgmt/restapi_data_transport.h"
#include "logger/event_logger.h"
#include "utility/http_helper.h"

namespace reinforcement_learning {
  namespace m = model_management;
  namespace u = utility;

  int restapi_data_transport_create(m::i_data_transport** retval, const u::configuration& config, i_trace* trace_logger, api_status* status);
  int observation_sender_create(i_sender** retval, const u::configuration&, error_callback_fn*, i_trace* trace_logger, api_status* status);
  int interaction_sender_create(i_sender** retval, const u::configuration&, error_callback_fn*, i_trace* trace_logger, api_status* status);
  int decision_sender_create(i_sender** retval, const u::configuration&, error_callback_fn*, i_trace* trace_logger, api_status* status);

  void register_azure_factories() {
    data_transport_factory.register_type(value::AZURE_STORAGE_BLOB, restapi_data_transport_create);
    sender_factory.register_type(value::OBSERVATION_EH_SENDER, observation_sender_create);
    sender_factory.register_type(value::INTERACTION_EH_SENDER, interaction_sender_create);
    sender_factory.register_type(value::DECISION_EH_SENDER, decision_sender_create);
  }

  int restapi_data_transport_create(m::i_data_transport** retval, const u::configuration& config, i_trace* trace_logger, api_status* status) {
    const auto uri = config.get(name::MODEL_BLOB_URI, nullptr);
    if (uri == nullptr) {
      RETURN_ERROR(trace_logger, status, http_uri_not_provided);
    }
    auto pret = new m::restapi_data_transport(new http_client(uri, config), trace_logger);
    const auto scode = pret->check(status);
    if (scode != error_code::success) {
      delete pret;
      return scode;
    }
    *retval = pret;
    return error_code::success;
  }

  std::string build_eh_url(const char* eh_host, const char* eh_name) {
    std::string url;
    url.append("https://").append(eh_host).append("/").append(eh_name)
      .append("/messages?timeout=60&api-version=2014-01");
    return url;
  }

  int observation_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status) {
    const auto eh_host = cfg.get(name::OBSERVATION_EH_HOST, "localhost:8080");
    const auto eh_name = cfg.get(name::OBSERVATION_EH_NAME, "observation");
    const auto eh_url = build_eh_url(eh_host, eh_name);

    *retval = new eventhub_client(
      new http_client(eh_url.c_str(), cfg),
      eh_host,
      cfg.get(name::OBSERVATION_EH_KEY_NAME, ""),
      cfg.get(name::OBSERVATION_EH_KEY, ""),
      eh_name,
      cfg.get_int(name::OBSERVATION_EH_TASKS_LIMIT, 16),
      cfg.get_int(name::OBSERVATION_EH_MAX_HTTP_RETRIES, 4),
      trace_logger,
      error_cb);
    return error_code::success;
  }

  int interaction_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status) {
    const auto eh_host = cfg.get(name::INTERACTION_EH_HOST, "localhost:8080");
    const auto eh_name = cfg.get(name::INTERACTION_EH_NAME, "interaction");
    const auto eh_url = build_eh_url(eh_host, eh_name);

    *retval = new eventhub_client(
      new http_client(eh_url.c_str(), cfg),
      cfg.get(name::INTERACTION_EH_HOST, "localhost:8080"),
      cfg.get(name::INTERACTION_EH_KEY_NAME, ""),
      cfg.get(name::INTERACTION_EH_KEY, ""),
      cfg.get(name::INTERACTION_EH_NAME, "interaction"),
      cfg.get_int(name::INTERACTION_EH_TASKS_LIMIT, 16),
      cfg.get_int(name::INTERACTION_EH_MAX_HTTP_RETRIES, 4),
      trace_logger,
      error_cb);
    return error_code::success;
  }

  int decision_sender_create(i_sender** retval, const u::configuration& cfg, error_callback_fn* error_cb, i_trace* trace_logger, api_status* status) {
    const auto eh_host = cfg.get(name::INTERACTION_EH_HOST, "localhost:8080");
    const auto eh_name = cfg.get(name::INTERACTION_EH_NAME, "interaction");
    const auto eh_url = build_eh_url(eh_host, eh_name);

    *retval = new eventhub_client(
      new http_client(eh_url.c_str(), cfg),
      cfg.get(name::INTERACTION_EH_HOST, "localhost:8080"),
      cfg.get(name::INTERACTION_EH_KEY_NAME, ""),
      cfg.get(name::INTERACTION_EH_KEY, ""),
      cfg.get(name::INTERACTION_EH_NAME, "interaction"),
      cfg.get_int(name::INTERACTION_EH_TASKS_LIMIT, 16),
      cfg.get_int(name::INTERACTION_EH_MAX_HTTP_RETRIES, 4),
      trace_logger,
      error_cb);
    return error_code::success;
  }
}
