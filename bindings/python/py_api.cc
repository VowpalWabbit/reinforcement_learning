#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "config_utility.h"
#include "live_model.h"

#include <memory>
#include <stdexcept>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
namespace rl = reinforcement_learning;

class rl_exception : public std::exception {
public:
  rl_exception_internal(const char *what_arg, int error_code)
      : _message(what_arg), _error_code{error_code} {}

  const char *what() const noexcept override { return message.c_str(); }
  int error_code() const { return _error_code; }

private:
  int _error_code;
  std::string _message = "";
};

#define THROW_IF_FAIL(x)                                                       \
  do {                                                                         \
    int retval__LINE__ = (x);                                                  \
    if (retval__LINE__ != rl::error_code::success) {                           \
      throw rl_exception(status.get_error_msg(), status.get_error_code());     \
    }                                                                          \
  } while (0)

struct error_callback_context {
  std::function<void(int, const std::string &)> _callback;
};

void dispatch_error_internal(const rl::api_status &status,
                             error_callback_context *context) {
  py::gil_scoped_acquire acquire;
  context->_callback(status.get_error_code(), status.get_error_msg());
}

class live_model_with_callback : public rl::live_model {
private:
  error_callback_context _context;

public:
  live_model_with_callback(
      const rl::utility::configuration &config,
      std::function<void(int, const std::string &)> callback)
      : rl::live_model(config, &dispatch_error_internal, &_context) {
    _context._callback = callback;
  }
};

PYBIND11_MODULE(rlclient_py, m) {
  static py::exception<rl_exception> ex(m, "RLException");

  py::class_<rl::utility::configuration>(m, "Configuration")
      .def(py::init<>())
      .def("get", &rl::utility::configuration::get)
      .def("set", &rl::utility::configuration::set);

  py::class_<rl::live_model>(m, "LiveModel")
      // TODO allow custom callback
      .def(py::init([](const rl::utility::configuration &config) {
        auto live_model =
            std::unique_ptr<rl::live_model>(new rl::live_model(config));
        rl::api_status status;
        THROW_IF_FAIL(live_model->init(&status));
        return live_model;
      }))
      .def(py::init([](const rl::utility::configuration &config,
                       std::function<void(int, const std::string &)> callback) {
        auto live_model = std::unique_ptr<live_model_with_callback>(
            new live_model_with_callback(config, callback));
        rl::api_status status;
        THROW_IF_FAIL(live_model->init(&status));
        return live_model;
      }))
      .def("choose_rank",
           [](rl::live_model &lm, const char *event_id,
              const char *context_json, bool deferred) {
             rl::ranking_response response;
             rl::api_status status;
             unsigned int flags = deferred ? rl::action_flags::DEFERRED
                                           : rl::action_flags::DEFAULT;
             THROW_IF_FAIL(lm.choose_rank(event_id, context_json, flags,
                                          response, &status));
             return response;
           })
      .def("choose_rank",
           [](rl::live_model &lm, const char *context_json, bool deferred) {
             rl::ranking_response response;
             unsigned int flags = deferred ? rl::action_flags::DEFERRED
                                           : rl::action_flags::DEFAULT;
             rl::api_status status;
             THROW_IF_FAIL(
                 lm.choose_rank(context_json, flags, response, &status));
             return response;
           })
      .def("report_action_taken",
           [](rl::live_model &lm, const char *event_id) {
             rl::api_status status;
             THROW_IF_FAIL(lm.report_action_taken(event_id, &status));
           })
      .def("report_outcome",
           [](rl::live_model &lm, const char *event_id, const char *outcome) {
             rl::api_status status;
             THROW_IF_FAIL(lm.report_outcome(event_id, outcome, &status));
           })
      .def("report_outcome",
           [](rl::live_model &lm, const char *event_id, float outcome) {
             rl::api_status status;
             THROW_IF_FAIL(lm.report_outcome(event_id, outcome, &status));
           })
      .def("refresh_model", [](rl::live_model &lm) {
        rl::api_status status;
        THROW_IF_FAIL(lm.refresh_model(&status));
      });

  py::class_<rl::ranking_response>(m, "RankingResponse")
      .def_property_readonly(
          "event_id",
          [](const rl::ranking_response &a) { return a.get_event_id(); })
      .def_property_readonly("chosen_action_id",
                             [](const rl::ranking_response &a) {
                               size_t chosen_action;
                               // todo handle failure
                               a.get_chosen_action_id(chosen_action);
                               return chosen_action;
                             })
      .def_property_readonly(
          "model_id",
          [](const rl::ranking_response &a) { return a.get_model_id(); })
      .def_property_readonly(
          "actions_probabilities", [](const rl::ranking_response &a) {
            py::list return_values;
            for (const auto &action_prob : a) {
              return_values.append(py::make_tuple(action_prob.action_id,
                                                  action_prob.probability));
            }
            return return_values;
          });

  m.def("create_config_from_json", [](const std::string &config_json) {
    rl::utility::configuration config;
    rl::api_status status;
    THROW_IF_FAIL(rl::utility::config::create_from_json(config_json, config,
                                                        nullptr, &status));
    return config;
  });

  py::module constants = m.def_submodule("constants");
  py::module prop = constants.def_submodule("prop");
  prop.attr("APP_ID") = py::str(rl::name::APP_ID);
  prop.attr("MODEL_SRC") = py::str(rl::name::MODEL_SRC);
  prop.attr("MODEL_BLOB_URI") = py::str(rl::name::MODEL_BLOB_URI);
  prop.attr("MODEL_REFRESH_INTERVAL_MS") =
      py::str(rl::name::MODEL_REFRESH_INTERVAL_MS);
  prop.attr("MODEL_IMPLEMENTATION") = py::str(rl::name::MODEL_IMPLEMENTATION);
  prop.attr("MODEL_BACKGROUND_REFRESH") =
      py::str(rl::name::MODEL_BACKGROUND_REFRESH);
  prop.attr("MODEL_VW_INITIAL_COMMAND_LINE") =
      py::str(rl::name::MODEL_VW_INITIAL_COMMAND_LINE);
  prop.attr("VW_CMDLINE") = py::str(rl::name::VW_CMDLINE);
  prop.attr("VW_POOL_INIT_SIZE") = py::str(rl::name::VW_POOL_INIT_SIZE);
  prop.attr("INITIAL_EPSILON") = py::str(rl::name::INITIAL_EPSILON);
  prop.attr("LEARNING_MODE") = py::str(rl::name::LEARNING_MODE);
  prop.attr("PROTOCOL_VERSION") = py::str(rl::name::PROTOCOL_VERSION);
  prop.attr("INTERACTION_EH_HOST") = py::str(rl::name::INTERACTION_EH_HOST);
  prop.attr("INTERACTION_EH_NAME") = py::str(rl::name::INTERACTION_EH_NAME);
  prop.attr("INTERACTION_EH_KEY_NAME") =
      py::str(rl::name::INTERACTION_EH_KEY_NAME);
  prop.attr("INTERACTION_EH_KEY") = py::str(rl::name::INTERACTION_EH_KEY);
  prop.attr("INTERACTION_EH_TASKS_LIMIT") =
      py::str(rl::name::INTERACTION_EH_TASKS_LIMIT);
  prop.attr("INTERACTION_EH_MAX_HTTP_RETRIES") =
      py::str(rl::name::INTERACTION_EH_MAX_HTTP_RETRIES);
  prop.attr("INTERACTION_SEND_HIGH_WATER_MARK") =
      py::str(rl::name::INTERACTION_SEND_HIGH_WATER_MARK);
  prop.attr("INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB") =
      py::str(rl::name::INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB);
  prop.attr("INTERACTION_SEND_BATCH_INTERVAL_MS") =
      py::str(rl::name::INTERACTION_SEND_BATCH_INTERVAL_MS);
  prop.attr("INTERACTION_SENDER_IMPLEMENTATION") =
      py::str(rl::name::INTERACTION_SENDER_IMPLEMENTATION);
  prop.attr("INTERACTION_USE_COMPRESSION") =
      py::str(rl::name::INTERACTION_USE_COMPRESSION);
  prop.attr("INTERACTION_USE_DEDUP") = py::str(rl::name::INTERACTION_USE_DEDUP);
  prop.attr("INTERACTION_QUEUE_MODE") =
      py::str(rl::name::INTERACTION_QUEUE_MODE);
  prop.attr("OBSERVATION_EH_HOST") = py::str(rl::name::OBSERVATION_EH_HOST);
  prop.attr("OBSERVATION_EH_NAME") = py::str(rl::name::OBSERVATION_EH_NAME);
  prop.attr("OBSERVATION_EH_KEY_NAME") =
      py::str(rl::name::OBSERVATION_EH_KEY_NAME);
  prop.attr("OBSERVATION_EH_KEY") = py::str(rl::name::OBSERVATION_EH_KEY);
  prop.attr("OBSERVATION_EH_TASKS_LIMIT") =
      py::str(rl::name::OBSERVATION_EH_TASKS_LIMIT);
  prop.attr("OBSERVATION_EH_MAX_HTTP_RETRIES") =
      py::str(rl::name::OBSERVATION_EH_MAX_HTTP_RETRIES);
  prop.attr("OBSERVATION_SEND_HIGH_WATER_MARK") =
      py::str(rl::name::OBSERVATION_SEND_HIGH_WATER_MARK);
  prop.attr("OBSERVATION_SEND_QUEUE_MAX_CAPACITY_KB") =
      py::str(rl::name::OBSERVATION_SEND_QUEUE_MAX_CAPACITY_KB);
  prop.attr("OBSERVATION_SEND_BATCH_INTERVAL_MS") =
      py::str(rl::name::OBSERVATION_SEND_BATCH_INTERVAL_MS);
  prop.attr("OBSERVATION_SENDER_IMPLEMENTATION") =
      py::str(rl::name::OBSERVATION_SENDER_IMPLEMENTATION);
  prop.attr("OBSERVATION_USE_COMPRESSION") =
      py::str(rl::name::OBSERVATION_USE_COMPRESSION);
  prop.attr("OBSERVATION_QUEUE_MODE") =
      py::str(rl::name::OBSERVATION_QUEUE_MODE);
  prop.attr("SEND_HIGH_WATER_MARK") = py::str(rl::name::SEND_HIGH_WATER_MARK);
  prop.attr("SEND_QUEUE_MAX_CAPACITY_KB") =
      py::str(rl::name::SEND_QUEUE_MAX_CAPACITY_KB);
  prop.attr("SEND_BATCH_INTERVAL_MS") =
      py::str(rl::name::SEND_BATCH_INTERVAL_MS);
  prop.attr("USE_COMPRESSION") = py::str(rl::name::USE_COMPRESSION);
  prop.attr("USE_DEDUP") = py::str(rl::name::USE_DEDUP);
  prop.attr("QUEUE_MODE") = py::str(rl::name::QUEUE_MODE);
  prop.attr("EH_TEST") = py::str(rl::name::EH_TEST);
  prop.attr("TRACE_LOG_IMPLEMENTATION") =
      py::str(rl::name::TRACE_LOG_IMPLEMENTATION);
  prop.attr("INTERACTION_FILE_NAME") = py::str(rl::name::INTERACTION_FILE_NAME);
  prop.attr("OBSERVATION_FILE_NAME") = py::str(rl::name::OBSERVATION_FILE_NAME);
  prop.attr("TIME_PROVIDER_IMPLEMENTATION") =
      py::str(rl::name::TIME_PROVIDER_IMPLEMENTATION);
  prop.attr("HTTP_CLIENT_DISABLE_CERT_VALIDATION") =
      py::str(rl::name::HTTP_CLIENT_DISABLE_CERT_VALIDATION);
  prop.attr("HTTP_CLIENT_TIMEOUT") = py::str(rl::name::HTTP_CLIENT_TIMEOUT);
  prop.attr("MODEL_FILE_NAME") = py::str(rl::name::MODEL_FILE_NAME);
  prop.attr("MODEL_FILE_MUST_EXIST") = py::str(rl::name::MODEL_FILE_MUST_EXIST);
  prop.attr("ZSTD_COMPRESSION_LEVEL") =
      py::str(rl::name::ZSTD_COMPRESSION_LEVEL);

  py::module value = constants.def_submodule("value");
  value.attr("AZURE_STORAGE_BLOB") = rl::value::AZURE_STORAGE_BLOB;
  value.attr("NO_MODEL_DATA") = rl::value::NO_MODEL_DATA;
  value.attr("FILE_MODEL_DATA") = rl::value::FILE_MODEL_DATA;
  value.attr("VW") = rl::value::VW;
  value.attr("PASSTHROUGH_PDF_MODEL") = rl::value::PASSTHROUGH_PDF_MODEL;
  value.attr("OBSERVATION_EH_SENDER") = rl::value::OBSERVATION_EH_SENDER;
  value.attr("INTERACTION_EH_SENDER") = rl::value::INTERACTION_EH_SENDER;
  value.attr("OBSERVATION_FILE_SENDER") = rl::value::OBSERVATION_FILE_SENDER;
  value.attr("INTERACTION_FILE_SENDER") = rl::value::INTERACTION_FILE_SENDER;
  value.attr("NULL_TRACE_LOGGER") = rl::value::NULL_TRACE_LOGGER;
  value.attr("CONSOLE_TRACE_LOGGER") = rl::value::CONSOLE_TRACE_LOGGER;
  value.attr("NULL_TIME_PROVIDER") = rl::value::NULL_TIME_PROVIDER;
  value.attr("CLOCK_TIME_PROVIDER") = rl::value::CLOCK_TIME_PROVIDER;
  value.attr("LEARNING_MODE_ONLINE") = rl::value::LEARNING_MODE_ONLINE;
  value.attr("LEARNING_MODE_APPRENTICE") = rl::value::LEARNING_MODE_APPRENTICE;
  value.attr("LEARNING_MODE_LOGGINGONLY") =
      rl::value::LEARNING_MODE_LOGGINGONLY;
  value.attr("CONTENT_ENCODING_IDENTITY") =
      rl::value::CONTENT_ENCODING_IDENTITY;
  value.attr("CONTENT_ENCODING_DEDUP") = rl::value::CONTENT_ENCODING_DEDUP;
  value.attr("QUEUE_MODE_DROP") = rl::value::QUEUE_MODE_DROP;
  value.attr("QUEUE_MODE_BLOCK") = rl::value::QUEUE_MODE_BLOCK;
}
