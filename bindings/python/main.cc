#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "live_model.h"

#include <memory>
#include <stdexcept>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;
namespace rl = reinforcement_learning;

#define THROW_IF_FAIL(x)                                                       \
  do {                                                                         \
    int retval__LINE__ = (x);                                                  \
    if (retval__LINE__ != rl::error_code::success) {                           \
      throw std::runtime_error(status.get_error_msg());                        \
    }                                                                          \
  } while (0)

struct error_callback_context {
  std::function<void(int, const std::string &)> _callback;
};

void dispatch_error_internal(const reinforcement_learning::api_status &status,
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
  py::class_<rl::utility::configuration>(m, "configuration")
      .def(py::init<>())
      .def("get", &rl::utility::configuration::get)
      .def("set", &rl::utility::configuration::set);

  py::class_<rl::live_model>(m, "live_model")
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
             reinforcement_learning::api_status status;
             THROW_IF_FAIL(lm.report_action_taken(event_id, &status));
           })
      .def("report_outcome",
           [](rl::live_model &lm, const char *event_id, const char *outcome) {
             reinforcement_learning::api_status status;
             THROW_IF_FAIL(lm.report_outcome(event_id, outcome, &status));
           })
      .def("report_outcome",
           [](rl::live_model &lm, const char *event_id, float outcome) {
             reinforcement_learning::api_status status;
             THROW_IF_FAIL(lm.report_outcome(event_id, outcome, &status));
           })
      .def("refresh_model", [](rl::live_model &lm) {
        reinforcement_learning::api_status status;
        THROW_IF_FAIL(lm.refresh_model(&status));
      });

  py::class_<rl::ranking_response>(m, "ranking_response")
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
      .def_property_readonly("action_ids",
                             [](const rl::ranking_response &a) {
                               py::list return_values;
                               for (const auto &action_prob : a) {
                                 return_values.append(action_prob.action_id);
                               }
                               return return_values;
                             })
      .def_property_readonly("probabilities",
                             [](const rl::ranking_response &a) {
                               py::list return_values;
                               for (const auto &action_prob : a) {
                                 return_values.append(action_prob.probability);
                               }
                               return return_values;
                             });
}
