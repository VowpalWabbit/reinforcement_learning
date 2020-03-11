#include "rl_logger_impl.h"

#include "utility/context_helper.h"
#include "sender.h"
#include "api_status.h"
#include "configuration.h"
#include "error_callback_fn.h"
#include "ranking_response.h"

#include "err_constants.h"
#include "constants.h"
#include "trace_logger.h"
#include "factory_resolver.h"
#include "logger/preamble_sender.h"
#include "utility/invariant_helpers.h"

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

// Some namespace changes for more concise code
using namespace std;

namespace reinforcement_learning {
  // Some namespace changes for more concise code
  namespace u = utility;
  namespace l = logger;
}

namespace reinforcement_learning {
  int rl_logger_impl::init(api_status* status) {
    if (!_trace_logger_init) {
      RETURN_IF_FAIL(init_trace(status));
    }
    RETURN_IF_FAIL(init_loggers(status));
    return error_code::success;
  }

  int rl_logger_impl::report_decision(const char* context, unsigned int flags, learning_mode _learning_mode, const ranking_response& response,
    api_status* status) {
    //clear previous errors if any
    api_status::try_clear(status);

    RETURN_IF_FAIL(_ranking_logger->log(context, flags, _learning_mode, response, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog->has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }

  int rl_logger_impl::report_decision(const char* context, unsigned int flags, const decision_response& response, api_status* status) {
    //clear previous errors if any
    api_status::try_clear(status);

    RETURN_IF_FAIL(_ccb_logger->log(context, flags, response, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog->has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }

  int rl_logger_impl::report_action_taken(const char* event_id, api_status* status) {
    // Clear previous errors if any
    api_status::try_clear(status);
    // Send the outcome event to the backend
    return _outcome_logger->report_action_taken(event_id, status);
  }

  int rl_logger_impl::report_outcome(const char* event_id, const char* outcome, api_status* status) {
    RETURN_IF_FAIL(utility::check_null_or_empty(event_id, outcome, status));
    return report_outcome_internal(event_id, outcome, status);
  }

  int rl_logger_impl::report_outcome(const char* event_id, float outcome, api_status* status) {
    RETURN_IF_FAIL(utility::check_null_or_empty(event_id, status));
    return report_outcome_internal(event_id, outcome, status);
  }

  rl_logger_impl::rl_logger_impl(
    const utility::configuration& config,
    error_fn fn,
    void* err_context,
    trace_logger_factory_t* trace_factory,
    sender_factory_t* sender_factory,
    time_provider_factory_t* time_provider_factory
  )
    : _configuration(config)
    , _error_cb(new error_callback_fn(fn, err_context))
    , _watchdog(new utility::watchdog(_error_cb.get()))
    , _trace_factory(trace_factory)
    , _sender_factory(sender_factory)
    , _time_provider_factory{ time_provider_factory }
  {
    // If there is no user supplied error callback, supply a default one that does nothing but report unhandled background errors.
    if (fn == nullptr) {
      _error_cb->set(&default_error_callback, &_watchdog);
    }
  }

  rl_logger_impl::rl_logger_impl(
    const utility::configuration& config,
    const std::shared_ptr<error_callback_fn>& error_cb,
    const std::shared_ptr<utility::watchdog>& watchdog,
    const std::shared_ptr<i_trace>& trace_logger,
    sender_factory_t* sender_factory,
    time_provider_factory_t* time_provider_factory)
  : _configuration(config)
  , _error_cb(error_cb)
  , _watchdog(watchdog)
  , _trace_logger(trace_logger)
  , _sender_factory(sender_factory)
  , _time_provider_factory(time_provider_factory)
  , _trace_logger_init(true)
  {
  }

  int rl_logger_impl::init_trace(api_status* status) {
    const auto trace_impl = _configuration.get(name::TRACE_LOG_IMPLEMENTATION, value::NULL_TRACE_LOGGER);
    i_trace* plogger;
    RETURN_IF_FAIL(_trace_factory->create(&plogger, trace_impl, _configuration, nullptr, status));
    _trace_logger.reset(plogger);
    TRACE_INFO(_trace_logger, "API Tracing initialized");
    _watchdog->set_trace_log(_trace_logger.get());
    return error_code::success;
  }

  int rl_logger_impl::init_loggers(api_status* status) {
    // Get the name of raw data (as opposed to message) sender for interactions.
    const auto ranking_sender_impl = _configuration.get(name::INTERACTION_SENDER_IMPLEMENTATION, value::INTERACTION_EH_SENDER);
    i_sender* ranking_data_sender;

    // Use the name to create an instance of raw data sender for interactions
    RETURN_IF_FAIL(_sender_factory->create(&ranking_data_sender, ranking_sender_impl, _configuration, _error_cb.get(), _trace_logger.get(), status));
    RETURN_IF_FAIL(ranking_data_sender->init(status));

    // Create a message sender that will prepend the message with a preamble and send the raw data using the
    // factory created raw data sender
    l::i_message_sender* ranking_msg_sender = new l::preamble_message_sender(ranking_data_sender);
    RETURN_IF_FAIL(ranking_msg_sender->init(status));

    // Get time provider factory and implementation
    const auto time_provider_impl = _configuration.get(name::TIME_PROVIDER_IMPLEMENTATION, value::NULL_TIME_PROVIDER);
    i_time_provider* interaction_time_provider;
    RETURN_IF_FAIL(_time_provider_factory->create(&interaction_time_provider, time_provider_impl, _configuration, _trace_logger.get(), status));

    // Create a logger for interactions that will use msg sender to send interaction messages
    _ranking_logger.reset(new logger::interaction_logger(_configuration, ranking_msg_sender, *_watchdog, interaction_time_provider, _error_cb.get()));
    RETURN_IF_FAIL(_ranking_logger->init(status));

    // Get the name of raw data (as opposed to message) sender for observations.
    const auto outcome_sender_impl = _configuration.get(name::OBSERVATION_SENDER_IMPLEMENTATION, value::OBSERVATION_EH_SENDER);
    i_sender* outcome_sender;

    // Use the name to create an instance of raw data sender for observations
    RETURN_IF_FAIL(_sender_factory->create(&outcome_sender, outcome_sender_impl, _configuration, _error_cb.get(), _trace_logger.get(), status));
    RETURN_IF_FAIL(outcome_sender->init(status));

    // Create a message sender that will prepend the message with a preamble and send the raw data using the
    // factory created raw data sender
    l::i_message_sender* outcome_msg_sender = new l::preamble_message_sender(outcome_sender);
    RETURN_IF_FAIL(outcome_msg_sender->init(status));

    // Get time provider implementation
    i_time_provider* observation_time_provider;
    RETURN_IF_FAIL(_time_provider_factory->create(&observation_time_provider, time_provider_impl, _configuration, _trace_logger.get(), status));

    // Create a logger for interactions that will use msg sender to send interaction messages
    _outcome_logger.reset(new logger::observation_logger(_configuration, outcome_msg_sender, *_watchdog, observation_time_provider, _error_cb.get()));
    RETURN_IF_FAIL(_outcome_logger->init(status));

    // Get the name of raw data (as opposed to message) sender for interactions.
    const auto decision_sender_impl = _configuration.get(name::DECISION_SENDER_IMPLEMENTATION, value::DECISION_EH_SENDER);
    i_sender* decision_data_sender;

    // Use the name to create an instance of raw data sender for interactions
    RETURN_IF_FAIL(_sender_factory->create(&decision_data_sender, decision_sender_impl, _configuration, _error_cb.get(), _trace_logger.get(), status));
    RETURN_IF_FAIL(decision_data_sender->init(status));

    // Create a message sender that will prepend the message with a preamble and send the raw data using the
    // factory created raw data sender
    l::i_message_sender* decision_msg_sender = new l::preamble_message_sender(decision_data_sender);
    RETURN_IF_FAIL(decision_msg_sender->init(status));

    i_time_provider* decision_time_provider;
    RETURN_IF_FAIL(_time_provider_factory->create(&decision_time_provider, time_provider_impl, _configuration, _trace_logger.get(), status));

    // Create a logger for interactions that will use msg sender to send interaction messages
    _ccb_logger.reset(new logger::ccb_logger(_configuration, decision_msg_sender, *_watchdog, decision_time_provider, _error_cb.get()));
    RETURN_IF_FAIL(_ccb_logger->init(status));

    return error_code::success;
  }
}
