#pragma once

#include "api_status.h"
#include "configuration.h"
#include "constants.h"
#include "learning_mode.h"
#include "ranking_response.h"
#include "../error_callback_fn.h"
#include "utility/watchdog.h"

#include "message_sender.h"
#include "time_helper.h"


namespace reinforcement_learning
{
  namespace logger {
    class cb_logger {
    public:
      cb_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr);
      
      cb_logger(const cb_logger& other) = delete;
      cb_logger& operator=(const cb_logger& other) = delete;
      cb_logger(cb_logger&& other) = delete;
      cb_logger& operator=(cb_logger&& other) = delete;

      ~cb_logger() = default;

      int log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode = ONLINE);
    };

    class ccb_logger {
    public:
      ccb_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr);

      ccb_logger(const ccb_logger& other) = delete;
      ccb_logger& operator=(const ccb_logger& other) = delete;
      ccb_logger(ccb_logger&& other) = delete;
      ccb_logger& operator=(ccb_logger&& other) = delete;

      ~ccb_logger() = default;

      int log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);
    };

    class slates_logger {
      slates_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr);

      slates_logger(const slates_logger& other) = delete;
      slates_logger& operator=(const slates_logger& other) = delete;
      slates_logger(slates_logger&& other) = delete;
      slates_logger& operator=(slates_logger&& other) = delete;

      ~slates_logger() = default;

      int log_decision(const std::string& event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);
    };

    class outcome_logger {
    public:
      outcome_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr);

      outcome_logger(const outcome_logger& other) = delete;
      outcome_logger& operator=(const outcome_logger& other) = delete;
      outcome_logger(outcome_logger&& other) = delete;
      outcome_logger& operator=(outcome_logger&& other) = delete;

      ~outcome_logger() = default;

      int log(const char* event_id, float outcome, api_status* status);

      int log(const char* event_id, const char* outcome, api_status* status);

      int report_action_taken(const char* event_id, api_status* status);
    };
  }
}