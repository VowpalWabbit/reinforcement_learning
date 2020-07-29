#include "logger_facade.h"
#include "err_constants.h"

namespace reinforcement_learning {
  namespace logger {
    int api_not_supported(api_status* status) {
      api_status::try_update(status, error_code::api_not_supported,
        "Current API version is not supported");
      return error_code::api_not_supported;
    }

    cb_logger_facade::cb_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : version(c.get_int(name::API_VERSION, 1))
    , v1(version == 1 ? new interaction_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr) {
    }

    int cb_logger_facade::init(api_status* status) {
      switch (version) {
        case 1: return v1->init(status);
        default: return api_not_supported(status);
      }
    }

    int cb_logger_facade::log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode) {
      switch (version) {
        case 1: return v1->log(event_id, context, flags, response, status, learning_mode);
        default: return api_not_supported(status);
      }
    }

    ccb_logger_facade::ccb_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : version(c.get_int(name::API_VERSION, 1))
    , v1(version == 1 ? new ccb_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr) {
    }

    int ccb_logger_facade::init(api_status* status) {
      switch (version) {
        case 1: return v1->init(status);
        default: return api_not_supported(status);
      }
    }

    int ccb_logger_facade::log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      switch (version) {
        case 1: return v1->log_decisions(event_ids, context, flags, action_ids, pdfs, model_version, status);
        default: return api_not_supported(status);
      }
    }

    slates_logger_facade::slates_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : version(c.get_int(name::API_VERSION, 1))
    , v1(version == 1 ? new slates_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr) {
    }

    int slates_logger_facade::init(api_status* status) {
      switch (version) {
        case 1: return v1->init(status);
        default: return api_not_supported(status);
      }
    }

    int slates_logger_facade::log_decision(const std::string& event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      switch (version) {
        case 1: return v1->log_decision(event_id, context, flags, action_ids, pdfs, model_version, status);
        default: return api_not_supported(status);
      }
    }

    outcome_logger_facade::outcome_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : version(c.get_int(name::API_VERSION, 1))
    , v1(version == 1 ? new observation_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr) {
    }

    int outcome_logger_facade::init(api_status* status) {
      switch (version) {
        case 1: return v1->init(status);
        default: return api_not_supported(status);
      }
    }

    int outcome_logger_facade::log(const char* event_id, float outcome, api_status* status) {
      switch (version) {
        case 1: return v1->log(event_id, outcome, status);
        default: return api_not_supported(status);
      }
    }

    int outcome_logger_facade::log(const char* event_id, const char* outcome, api_status* status) {
      switch (version) {
        case 1: return v1->log(event_id, outcome, status);
        default: return api_not_supported(status);
      }
    }

    int outcome_logger_facade::report_action_taken(const char* event_id, api_status* status) {
      switch (version) {
        case 1: return v1->report_action_taken(event_id, status);
        default: return api_not_supported(status);
      }
    }
  }
}