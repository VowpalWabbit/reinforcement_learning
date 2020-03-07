#pragma once
#include "logger/event_logger.h"
#include "utility/periodic_background_proc.h"
#include "factory_resolver.h"
#include "error_callback_fn.h"
#include "trace_logger.h"

#include "utility/watchdog.h"

#include <atomic>
#include <memory>

namespace reinforcement_learning
{
  class ranking_response;
  class decision_response;
  class api_status;
}

namespace reinforcement_learning
{
  class rl_logger_impl {
  public:
    using error_fn = void(*)(const api_status&, void* user_context);

    int init(api_status* status);

    int report_decision(const char* context, unsigned int flags, const ranking_response& response, api_status* status);
    int report_decision(const char* context, unsigned int flags, const decision_response& response, api_status* status);

    int report_action_taken(const char* event_id, api_status* status);

    int report_outcome(const char* event_id, const char* outcome_data, api_status* status);
    int report_outcome(const char* event_id, float reward, api_status* status);

    rl_logger_impl(
      const utility::configuration& config,
      error_fn fn,
      void* err_context,
      trace_logger_factory_t* trace_factory,
      sender_factory_t* sender_factory,
      time_provider_factory_t* time_provider_factory);

    rl_logger_impl(
      const utility::configuration& config,
      const std::shared_ptr<error_callback_fn>& error_cb,
      const std::shared_ptr<utility::watchdog>& watchdog,
      const std::shared_ptr<i_trace>& trace_logger,
      sender_factory_t* sender_factory,
      time_provider_factory_t* time_provider_factory);

    rl_logger_impl(const rl_logger_impl&) = delete;
    rl_logger_impl(rl_logger_impl&&) = delete;
    rl_logger_impl& operator=(const rl_logger_impl&) = delete;
    rl_logger_impl& operator=(rl_logger_impl&&) = delete;

  private:
    // Internal implementation methods
    int init_loggers(api_status* status);
    int init_trace(api_status* status);
    template<typename D>
    int report_outcome_internal(const char* event_id, D outcome, api_status* status);

  private:
    // Internal implementation state
    utility::configuration _configuration;
    std::shared_ptr<error_callback_fn> _error_cb;
    std::shared_ptr<utility::watchdog> _watchdog;

    trace_logger_factory_t* _trace_factory;
    sender_factory_t* _sender_factory;
    time_provider_factory_t* _time_provider_factory;

    std::unique_ptr<reinforcement_learning::logger::interaction_logger> _ranking_logger{ nullptr };
    std::unique_ptr<reinforcement_learning::logger::observation_logger> _outcome_logger{ nullptr };
    std::unique_ptr<reinforcement_learning::logger::ccb_logger> _ccb_logger{ nullptr };
    std::shared_ptr<i_trace> _trace_logger{ nullptr };
    bool _trace_logger_init{ false };
  };

  template <typename D>
  int rl_logger_impl::report_outcome_internal(const char* event_id, D outcome, api_status* status) {
    // Clear previous errors if any
    api_status::try_clear(status);

    // Send the outcome event to the backend
    RETURN_IF_FAIL(_outcome_logger->log(event_id, outcome, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog->has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }
}
