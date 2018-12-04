#pragma once

#include <vector>
#include <string>
#include <exception>

#include "configuration.h"
#include "live_model.h"

namespace reinforcement_learning {
  namespace python {

    reinforcement_learning::utility::configuration create_config_from_json(const std::string& config_json);

    class error_callback {
    public:
      virtual void on_error(int error_code, const std::string& error_message) {}
      virtual ~error_callback() = default;
    };

    struct ranking_response {
      std::string event_id;
      std::string model_id;
      size_t chosen_action_id;
      std::vector<int> action_ids;
      std::vector<float> probabilities;
    };

#ifndef SWIG
    class rl_exception_internal : public std::runtime_error
    {
      public:
        rl_exception_internal(const std::string& what_arg, int error_code)
          : std::runtime_error(what_arg), _error_code{error_code}
        {}
        rl_exception_internal(const char* what_arg, int error_code)
          : std::runtime_error(what_arg), _error_code{error_code}
        {}

        int error_code() const { return _error_code; }

      private:
        int _error_code;
    };

    // This is defined in the interface file instead so that custom Python can be injected
    class live_model {
    public:
      live_model(const reinforcement_learning::utility::configuration config, error_callback& callback);
      live_model(const reinforcement_learning::utility::configuration config);

      void init();

      ranking_response choose_rank(const char* event_id, const char* context_json, bool deferred);
      // event_id is auto-generated.
      ranking_response choose_rank(const char* context_json, bool deferred);

      void report_action_taken(const char* event_id);

      void report_outcome(const char* event_id, const char* outcome);
      void report_outcome(const char* event_id, float outcome);

      private:
        reinforcement_learning::live_model _impl;
    };
#endif
  }
}
