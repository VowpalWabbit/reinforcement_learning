#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "config_utility.h"
#include "constants.h"
#include "live_model.h"
#include "multistep.h"

#include <exception>
#include <memory>
#include <cstring>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
namespace rl = reinforcement_learning;

class rl_exception : public std::exception {
public:
  rl_exception(const char *what_arg, int error_code)
      : _message(what_arg), _error_code{error_code} {}

  const char *what() const noexcept override { return _message.c_str(); }
  int error_code() const { return _error_code; }

private:
  int _error_code;
  std::string _message;
};

// Excellent blog post about how to get custom exception types working with
// PyBind11
// https://www.pierov.org/2020/03/01/python-custom-exceptions-c-extensions/
static PyObject *RLException_tp_str(PyObject *self_ptr) {
  py::str ret;
  try {
    py::handle self(self_ptr);
    py::tuple args = self.attr("args");
    ret = py::str(args[0]);
  } catch (py::error_already_set &e) {
    ret = "";
  }

  /* ret will go out of scope when returning, therefore increase its reference
  count, and transfer it to the caller (like PyObject_Str). */
  ret.inc_ref();
  return ret.ptr();
}

static PyObject *RLException_getcode(PyObject *self_ptr, void *closure) {
  try {
    py::handle self(self_ptr);
    py::tuple args = self.attr("args");
    py::object code = args[1];
    code.inc_ref();
    return code.ptr();
  } catch (py::error_already_set &e) {
    /* We could simply backpropagate the exception with e.restore, but
    exceptions like OSError return None when an attribute is not set. */
    py::none ret;
    ret.inc_ref();
    return ret.ptr();
  }
}

static PyGetSetDef RLException_getsetters[] = {
    {"code", RLException_getcode, NULL,
     "Get the error code for this exception.", NULL},
    {NULL}};

static PyObject *PyRLException;

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

struct constants {
  constants() = delete;
};

PYBIND11_MODULE(rl_client, m) {
  PyRLException = PyErr_NewException("rl_client.RLException", NULL, NULL);
  if (PyRLException) {
    PyTypeObject *as_type = reinterpret_cast<PyTypeObject *>(PyRLException);
    as_type->tp_str = RLException_tp_str;
    PyObject *descr = PyDescr_NewGetSet(as_type, RLException_getsetters);
    auto dict = py::reinterpret_borrow<py::dict>(as_type->tp_dict);
    dict[py::handle(PyDescr_NAME(descr))] = py::handle(descr);

    Py_XINCREF(PyRLException);
    m.add_object("RLException", py::handle(PyRLException));
  }

  py::register_exception_translator([](std::exception_ptr p) {
    try {
      if (p) {
        std::rethrow_exception(p);
      }
    } catch (rl_exception &e) {
      py::tuple args(2);
      args[0] = e.what();
      args[1] = e.error_code();
      PyErr_SetObject(PyRLException, args.ptr());
    }
  });

  py::class_<rl::utility::configuration>(m, "Configuration", R"pbdoc(
        Container class for all configuration values. Generally is constructed from client.json file read from disk
    )pbdoc")
      .def(py::init<>())
      .def("get", &rl::utility::configuration::get,
           py::arg("name"),
           py::arg("defval"),
           R"pbdoc(
        Get a config value or default.

        :param name: Name of configuration value to get
        :param defval: Value to return if name does not exist
        :returns: string value from config
    )pbdoc")
      .def("set", &rl::utility::configuration::set);

  py::class_<rl::live_model>(m, "LiveModel")
      .def(py::init([](const rl::utility::configuration &config) {
             auto live_model =
                 std::unique_ptr<rl::live_model>(new rl::live_model(config));
             rl::api_status status;
             THROW_IF_FAIL(live_model->init(&status));
             return live_model;
           }),
           py::arg("config"))
      .def(py::init([](const rl::utility::configuration &config,
                       std::function<void(int, const std::string &)> callback) {
             auto live_model = std::unique_ptr<live_model_with_callback>(
                 new live_model_with_callback(config, callback));
             rl::api_status status;
             THROW_IF_FAIL(live_model->init(&status));
             return live_model;
           }),
           py::arg("config"),
           py::arg("callback"))
      .def(
          "choose_rank",
          [](rl::live_model &lm, const char* context, const char *event_id,
             bool deferred) {
            rl::ranking_response response;
            rl::api_status status;
            unsigned int flags = deferred ? rl::action_flags::DEFERRED
                                          : rl::action_flags::DEFAULT;
            THROW_IF_FAIL(
                lm.choose_rank(event_id, {context, std::strlen(context)}, flags, response, &status));
            return response;
          },
          py::arg("context"),
          py::arg("event_id"),
          py::arg("deferred") = false,
          R"pbdoc(
        Request prediction for given context and use the given event_id

        :rtype: :class:`rl_client.RankingResponse`
    )pbdoc")
      .def(
          "choose_rank",
          [](rl::live_model &lm, const char* context, bool deferred) {
            rl::ranking_response response;
            rl::api_status status;
            unsigned int flags = deferred ? rl::action_flags::DEFERRED
                                          : rl::action_flags::DEFAULT;
            THROW_IF_FAIL(lm.choose_rank({context, std::strlen(context)}, flags, response, &status));
            return response;
          },
          py::arg("context"),
          py::arg("deferred") = false,
          R"pbdoc(
        Request prediction for given context and let an event id be generated

        :rtype: :class:`rl_client.RankingResponse`
    )pbdoc")
      .def(
          "request_episodic_decision",
          [](rl::live_model &lm, const char* event_id, const char* previous_id, const char* context, rl::episode_state& episode) {
            rl::ranking_response response;
            rl::api_status status;
            THROW_IF_FAIL(lm.request_episodic_decision(event_id, previous_id, {context, std::strlen(context)}, response, episode, &status));
            return response;
          },
          py::arg("event_id"),
          py::arg("previous_id"),
          py::arg("context"),
          py::arg("episode"))
      .def(
          "report_action_taken",
          [](rl::live_model &lm, const char *event_id) {
            rl::api_status status;
            THROW_IF_FAIL(lm.report_action_taken(event_id, &status));
          },
          py::arg("event_id"))
      .def(
          "report_outcome",
          [](rl::live_model &lm, const char *event_id, const char *outcome) {
            rl::api_status status;
            THROW_IF_FAIL(lm.report_outcome(event_id, outcome, &status));
          },
          py::arg("event_id"),
          py::arg("outcome"))
      .def(
          "report_outcome",
          [](rl::live_model &lm, const char *event_id, float outcome) {
            rl::api_status status;
            THROW_IF_FAIL(lm.report_outcome(event_id, outcome, &status));
          },
          py::arg("event_id"),
          py::arg("outcome"))
      .def(
          "report_outcome",
          [](rl::live_model &lm, const char *episode_id, const char *event_id, float outcome) {
            rl::api_status status;
            THROW_IF_FAIL(lm.report_outcome(episode_id, event_id, outcome, &status));
          },
          py::arg("episode_id"),
          py::arg("event_id"),
          py::arg("outcome"))
      .def("refresh_model", [](rl::live_model &lm) {
        rl::api_status status;
        THROW_IF_FAIL(lm.refresh_model(&status));
      });

  py::class_<rl::ranking_response>(m, "RankingResponse")
      .def_property_readonly(
          "event_id",
          [](const rl::ranking_response &a) { return a.get_event_id(); },
          R"pbdoc(
        Either the supplied event ID or auto generated if none was given

        :rtype: str
    )pbdoc")
      .def_property_readonly(
          "chosen_action_id",
          [](const rl::ranking_response &a) {
            size_t chosen_action;
            // todo handle failure
            a.get_chosen_action_id(chosen_action);
            return chosen_action;
          },
          R"pbdoc(
        Action chosen

        :rtype: int
    )pbdoc")
      .def_property_readonly(
          "model_id",
          [](const rl::ranking_response &a) { return a.get_model_id(); },
          R"pbdoc(
        ID of model used to make this prediction

        :rtype: str
    )pbdoc")
      .def_property_readonly(
          "actions_probabilities",
          [](const rl::ranking_response &a) {
            py::list return_values;
            for (const auto &action_prob : a) {
              return_values.append(py::make_tuple(action_prob.action_id,
                                                  action_prob.probability));
            }
            return return_values;
          },
          R"pbdoc(
            The list of action ids and corresponding probabilities

            :rtype: list[(int,float)]
    )pbdoc");

  // TODO: Expose episode history API.
  py::class_<rl::episode_state>(m, "EpisodeState")
      .def(py::init<const char *>())
      .def_property_readonly(
          "episode_id",
          [](const rl::episode_state &episode) {
            return episode.get_episode_id();
          });

  m.def(
      "create_config_from_json",
      [](const std::string &config_json) {
        rl::utility::configuration config;
        rl::api_status status;
        THROW_IF_FAIL(rl::utility::config::create_from_json(config_json, config,
                                                            nullptr, &status));
        return config;
      },
      py::arg("config_json"), R"pbdoc(
        Parse the input as JSON and return an :class:`rl_client.Configuration` object which contains each field.

        :param config_json: JSON string to parse
        :returns: :class:`rl_client.Configuration` object containing parsed values
    )pbdoc");

  py::class_<constants>(m, "constants")
    .def_property_readonly_static("APP_ID", [](py::object /*self*/) { return rl::name::APP_ID; })
    .def_property_readonly_static("MODEL_SRC", [](py::object /*self*/) { return rl::name::MODEL_SRC; })
    .def_property_readonly_static("MODEL_BLOB_URI", [](py::object /*self*/) { return rl::name::MODEL_BLOB_URI; })
    .def_property_readonly_static("MODEL_REFRESH_INTERVAL_MS", [](py::object /*self*/) { return rl::name::MODEL_REFRESH_INTERVAL_MS; })
    .def_property_readonly_static("MODEL_IMPLEMENTATION", [](py::object /*self*/) { return rl::name::MODEL_IMPLEMENTATION; })
    .def_property_readonly_static("MODEL_BACKGROUND_REFRESH", [](py::object /*self*/) { return rl::name::MODEL_BACKGROUND_REFRESH; })
    .def_property_readonly_static("MODEL_VW_INITIAL_COMMAND_LINE", [](py::object /*self*/) { return rl::name::MODEL_VW_INITIAL_COMMAND_LINE; })
    .def_property_readonly_static("VW_CMDLINE", [](py::object /*self*/) { return rl::name::VW_CMDLINE; })
    .def_property_readonly_static("VW_POOL_INIT_SIZE", [](py::object /*self*/) { return rl::name::VW_POOL_INIT_SIZE; })
    .def_property_readonly_static("INITIAL_EPSILON", [](py::object /*self*/) { return rl::name::INITIAL_EPSILON; })
    .def_property_readonly_static("LEARNING_MODE", [](py::object /*self*/) { return rl::name::LEARNING_MODE; })
    .def_property_readonly_static("PROTOCOL_VERSION", [](py::object /*self*/) { return rl::name::PROTOCOL_VERSION; })
    .def_property_readonly_static("INTERACTION_EH_HOST", [](py::object /*self*/) { return rl::name::INTERACTION_EH_HOST; })
    .def_property_readonly_static("INTERACTION_EH_NAME", [](py::object /*self*/) { return rl::name::INTERACTION_EH_NAME; })
    .def_property_readonly_static("INTERACTION_EH_KEY_NAME", [](py::object /*self*/) { return rl::name::INTERACTION_EH_KEY_NAME; })
    .def_property_readonly_static("INTERACTION_EH_KEY", [](py::object /*self*/) { return rl::name::INTERACTION_EH_KEY; })
    .def_property_readonly_static("INTERACTION_EH_TASKS_LIMIT", [](py::object /*self*/) { return rl::name::INTERACTION_EH_TASKS_LIMIT; })
    .def_property_readonly_static("INTERACTION_EH_MAX_HTTP_RETRIES", [](py::object /*self*/) { return rl::name::INTERACTION_EH_MAX_HTTP_RETRIES; })
    .def_property_readonly_static("INTERACTION_SEND_HIGH_WATER_MARK", [](py::object /*self*/) { return rl::name::INTERACTION_SEND_HIGH_WATER_MARK; })
    .def_property_readonly_static("INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB", [](py::object /*self*/) { return rl::name::INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB; })
    .def_property_readonly_static("INTERACTION_SEND_BATCH_INTERVAL_MS", [](py::object /*self*/) { return rl::name::INTERACTION_SEND_BATCH_INTERVAL_MS; })
    .def_property_readonly_static("INTERACTION_SENDER_IMPLEMENTATION", [](py::object /*self*/) { return rl::name::INTERACTION_SENDER_IMPLEMENTATION; })
    .def_property_readonly_static("INTERACTION_USE_COMPRESSION", [](py::object /*self*/) { return rl::name::INTERACTION_USE_COMPRESSION; })
    .def_property_readonly_static("INTERACTION_USE_DEDUP", [](py::object /*self*/) { return rl::name::INTERACTION_USE_DEDUP; })
    .def_property_readonly_static("INTERACTION_QUEUE_MODE", [](py::object /*self*/) { return rl::name::INTERACTION_QUEUE_MODE; })
    .def_property_readonly_static("OBSERVATION_EH_HOST", [](py::object /*self*/) { return rl::name::OBSERVATION_EH_HOST; })
    .def_property_readonly_static("OBSERVATION_EH_NAME", [](py::object /*self*/) { return rl::name::OBSERVATION_EH_NAME; })
    .def_property_readonly_static("OBSERVATION_EH_KEY_NAME", [](py::object /*self*/) { return rl::name::OBSERVATION_EH_KEY_NAME; })
    .def_property_readonly_static("OBSERVATION_EH_KEY", [](py::object /*self*/) { return rl::name::OBSERVATION_EH_KEY; })
    .def_property_readonly_static("OBSERVATION_EH_TASKS_LIMIT", [](py::object /*self*/) { return rl::name::OBSERVATION_EH_TASKS_LIMIT; })
    .def_property_readonly_static("OBSERVATION_EH_MAX_HTTP_RETRIES", [](py::object /*self*/) { return rl::name::OBSERVATION_EH_MAX_HTTP_RETRIES; })
    .def_property_readonly_static("OBSERVATION_SEND_HIGH_WATER_MARK", [](py::object /*self*/) { return rl::name::OBSERVATION_SEND_HIGH_WATER_MARK; })
    .def_property_readonly_static("OBSERVATION_SEND_QUEUE_MAX_CAPACITY_KB", [](py::object /*self*/) { return rl::name::OBSERVATION_SEND_QUEUE_MAX_CAPACITY_KB; })
    .def_property_readonly_static("OBSERVATION_SEND_BATCH_INTERVAL_MS", [](py::object /*self*/) { return rl::name::OBSERVATION_SEND_BATCH_INTERVAL_MS; })
    .def_property_readonly_static("OBSERVATION_SENDER_IMPLEMENTATION", [](py::object /*self*/) { return rl::name::OBSERVATION_SENDER_IMPLEMENTATION; })
    .def_property_readonly_static("OBSERVATION_USE_COMPRESSION", [](py::object /*self*/) { return rl::name::OBSERVATION_USE_COMPRESSION; })
    .def_property_readonly_static("OBSERVATION_QUEUE_MODE", [](py::object /*self*/) { return rl::name::OBSERVATION_QUEUE_MODE; })
    .def_property_readonly_static("SEND_HIGH_WATER_MARK", [](py::object /*self*/) { return rl::name::SEND_HIGH_WATER_MARK; })
    .def_property_readonly_static("SEND_QUEUE_MAX_CAPACITY_KB", [](py::object /*self*/) { return rl::name::SEND_QUEUE_MAX_CAPACITY_KB; })
    .def_property_readonly_static("SEND_BATCH_INTERVAL_MS", [](py::object /*self*/) { return rl::name::SEND_BATCH_INTERVAL_MS; })
    .def_property_readonly_static("USE_COMPRESSION", [](py::object /*self*/) { return rl::name::USE_COMPRESSION; })
    .def_property_readonly_static("USE_DEDUP", [](py::object /*self*/) { return rl::name::USE_DEDUP; })
    .def_property_readonly_static("QUEUE_MODE", [](py::object /*self*/) { return rl::name::QUEUE_MODE; })
    .def_property_readonly_static("EH_TEST", [](py::object /*self*/) { return rl::name::EH_TEST; })
    .def_property_readonly_static("TRACE_LOG_IMPLEMENTATION", [](py::object /*self*/) { return rl::name::TRACE_LOG_IMPLEMENTATION; })
    .def_property_readonly_static("INTERACTION_FILE_NAME", [](py::object /*self*/) { return rl::name::INTERACTION_FILE_NAME; })
    .def_property_readonly_static("OBSERVATION_FILE_NAME", [](py::object /*self*/) { return rl::name::OBSERVATION_FILE_NAME; })
    .def_property_readonly_static("TIME_PROVIDER_IMPLEMENTATION", [](py::object /*self*/) { return rl::name::TIME_PROVIDER_IMPLEMENTATION; })
    .def_property_readonly_static("HTTP_CLIENT_DISABLE_CERT_VALIDATION", [](py::object /*self*/) { return rl::name::HTTP_CLIENT_DISABLE_CERT_VALIDATION; })
    .def_property_readonly_static("HTTP_CLIENT_TIMEOUT", [](py::object /*self*/) { return rl::name::HTTP_CLIENT_TIMEOUT; })
    .def_property_readonly_static("MODEL_FILE_NAME", [](py::object /*self*/) { return rl::name::MODEL_FILE_NAME; })
    .def_property_readonly_static("MODEL_FILE_MUST_EXIST", [](py::object /*self*/) { return rl::name::MODEL_FILE_MUST_EXIST; })
    .def_property_readonly_static("ZSTD_COMPRESSION_LEVEL", [](py::object /*self*/) { return rl::name::ZSTD_COMPRESSION_LEVEL; })
    .def_property_readonly_static("AZURE_STORAGE_BLOB", [](py::object /*self*/) { return rl::value::AZURE_STORAGE_BLOB; })
    .def_property_readonly_static("NO_MODEL_DATA", [](py::object /*self*/) { return rl::value::NO_MODEL_DATA; })
    .def_property_readonly_static("FILE_MODEL_DATA", [](py::object /*self*/) { return rl::value::FILE_MODEL_DATA; })
    .def_property_readonly_static("VW", [](py::object /*self*/) { return rl::value::VW; })
    .def_property_readonly_static("PASSTHROUGH_PDF_MODEL", [](py::object /*self*/) { return rl::value::PASSTHROUGH_PDF_MODEL; })
    .def_property_readonly_static("OBSERVATION_EH_SENDER", [](py::object /*self*/) { return rl::value::OBSERVATION_EH_SENDER; })
    .def_property_readonly_static("INTERACTION_EH_SENDER", [](py::object /*self*/) { return rl::value::INTERACTION_EH_SENDER; })
    .def_property_readonly_static("OBSERVATION_FILE_SENDER", [](py::object /*self*/) { return rl::value::OBSERVATION_FILE_SENDER; })
    .def_property_readonly_static("INTERACTION_FILE_SENDER", [](py::object /*self*/) { return rl::value::INTERACTION_FILE_SENDER; })
    .def_property_readonly_static("NULL_TRACE_LOGGER", [](py::object /*self*/) { return rl::value::NULL_TRACE_LOGGER; })
    .def_property_readonly_static("CONSOLE_TRACE_LOGGER", [](py::object /*self*/) { return rl::value::CONSOLE_TRACE_LOGGER; })
    .def_property_readonly_static("NULL_TIME_PROVIDER", [](py::object /*self*/) { return rl::value::NULL_TIME_PROVIDER; })
    .def_property_readonly_static("CLOCK_TIME_PROVIDER", [](py::object /*self*/) { return rl::value::CLOCK_TIME_PROVIDER; })
    .def_property_readonly_static("LEARNING_MODE_ONLINE", [](py::object /*self*/) { return rl::value::LEARNING_MODE_ONLINE; })
    .def_property_readonly_static("LEARNING_MODE_APPRENTICE", [](py::object /*self*/) { return rl::value::LEARNING_MODE_APPRENTICE; })
    .def_property_readonly_static("LEARNING_MODE_LOGGINGONLY", [](py::object /*self*/) { return rl::value::LEARNING_MODE_LOGGINGONLY; })
    .def_property_readonly_static("CONTENT_ENCODING_IDENTITY", [](py::object /*self*/) { return rl::value::CONTENT_ENCODING_IDENTITY; })
    .def_property_readonly_static("CONTENT_ENCODING_DEDUP", [](py::object /*self*/) { return rl::value::CONTENT_ENCODING_DEDUP; })
    .def_property_readonly_static("QUEUE_MODE_DROP", [](py::object /*self*/) { return rl::value::QUEUE_MODE_DROP; })
    .def_property_readonly_static("QUEUE_MODE_BLOCK", [](py::object /*self*/) { return rl::value::QUEUE_MODE_BLOCK; });
}
