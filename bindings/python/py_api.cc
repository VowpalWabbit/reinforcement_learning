#include "py_api.h"

#include "config_utility.h"
#include "configuration.h"
#include "factory_resolver.h"
#include "api_status.h"
#include "err_constants.h"

#include <Python.h>

#include <exception>
#include <sstream>

namespace reinforcement_learning {
  namespace python {

    // PyEval_InitThreads must be called before any threads are created, and definitely before we try to take the GIL.
    // https://stackoverflow.com/questions/5140998/why-does-pygilstate-release-segfault-in-this-case
    struct InitializeGILOnStartup {
      InitializeGILOnStartup(){
        PyEval_InitThreads();
      }
    } ensureGILIsReadyBeforeCreatingThreads;

    void dispatch_error_internal(const reinforcement_learning::api_status& status, error_callback* context) {
      // Obtain global interpreter lock to execute Python.
      PyGILState_STATE gstate;
      gstate = PyGILState_Ensure();

      // Callback defined by Python subclass.
      context->on_error(status.get_error_code(), status.get_error_msg());

      // Release the global interpreter lock.
      PyGILState_Release(gstate);
    }

    void check_api_status(const reinforcement_learning::api_status& status) {
      if (status.get_error_code() != reinforcement_learning::error_code::success) {
        std::stringstream ss;
        ss << "Error code: " << status.get_error_code() << ", Message: " << status.get_error_msg() << std::endl;
        throw std::runtime_error(ss.str());
      }
    }

    reinforcement_learning::utility::configuration create_config_from_json(const std::string& config_json) {
      reinforcement_learning::utility::configuration config;
      reinforcement_learning::api_status status;
      reinforcement_learning::utility::config::create_from_json(config_json, config, nullptr, &status);
      check_api_status(status);

      return config;
    }

    live_model::live_model(const reinforcement_learning::utility::configuration config, error_callback& callback)
      : _impl(config, &dispatch_error_internal, &callback, &trace_logger_factory, &data_transport_factory, &model_factory)
    {}

    live_model::live_model(const reinforcement_learning::utility::configuration config)
      : _impl(config, nullptr, nullptr, &trace_logger_factory, &data_transport_factory, &model_factory)
    {}

    void live_model::init() {
      reinforcement_learning::api_status status;
      _impl.init(&status);
      check_api_status(status);
    }

    ranking_response convert_ranking_response(const reinforcement_learning::ranking_response& response_impl) {
      ranking_response response;
      reinforcement_learning::api_status status;

      response.event_id = response_impl.get_event_id();
      response.model_id = response_impl.get_model_id();

      size_t chosen_action_id;
      response_impl.get_chosen_action_id(chosen_action_id, &status);
      check_api_status(status);
      response.chosen_action_id = chosen_action_id;

      std::vector<int> action_ids;
      std::vector<float> possibilities;
      action_ids.reserve(response_impl.size());
      possibilities.reserve(response_impl.size());

      for (const auto& action_prob : response_impl) {
        action_ids.push_back(action_prob.action_id);
        possibilities.push_back(action_prob.probability);
      }

      response.action_ids = std::move(action_ids);
      response.probabilities = std::move(possibilities);

      return response;
    }

    ranking_response live_model::choose_rank(const char* event_id, const char* context_json, bool deferred) {
      reinforcement_learning::ranking_response response_impl;
      reinforcement_learning::api_status status;
      unsigned int flags = deferred ? action_flags::DEFERRED : action_flags::DEFAULT;
      _impl.choose_rank(event_id, context_json, flags, response_impl, &status);
      check_api_status(status);

      return convert_ranking_response(response_impl);
    }
    // event_id is auto-generated.
    ranking_response live_model::choose_rank(const char* context_json, bool deferred) {
      reinforcement_learning::ranking_response response_impl;
      reinforcement_learning::api_status status;
      unsigned int flags = deferred ? action_flags::DEFERRED : action_flags::DEFAULT;
      _impl.choose_rank(context_json, flags, response_impl, &status);
      check_api_status(status);

      return convert_ranking_response(response_impl);
    }

    void live_model::report_action_taken(const char* event_id) {
      reinforcement_learning::api_status status;
      _impl.report_action_taken(event_id, &status);
      check_api_status(status);
    }

    void live_model::report_outcome(const char* event_id, const char* outcome) {
      reinforcement_learning::api_status status;
      _impl.report_outcome(event_id, outcome, &status);
      check_api_status(status);
    }

    void live_model::report_outcome(const char* event_id, float outcome) {
      reinforcement_learning::api_status status;
      _impl.report_outcome(event_id, outcome, &status);
      check_api_status(status);
    }
  }
}
