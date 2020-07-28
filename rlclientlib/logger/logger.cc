#include "logger.h"

namespace reinforcement_learning {
  namespace logger {
    cb_logger::cb_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb) {
      ;
    }

    int cb_logger::log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode) {
      return 0;
    }

    ccb_logger::ccb_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb) {
      ;
    }

    int ccb_logger::log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      return 0;
    }


    slates_logger::slates_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb) {
      ;
    }

    int slates_logger::log_decision(const std::string& event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      return 0;
    }


    outcome_logger::outcome_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    {
      ;
    }

    int outcome_logger::log(const char* event_id, float outcome, api_status* status) {
      return 0;
    }

    int outcome_logger::log(const char* event_id, const char* outcome, api_status* status) {
      return 0;
    }

    int outcome_logger::report_action_taken(const char* event_id, api_status* status) {
      return 0;
    }
  }
}