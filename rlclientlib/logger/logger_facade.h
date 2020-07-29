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

#include "event_logger.h"


namespace reinforcement_learning
{
  namespace logger {
    class cb_logger_facade {
    public:
      cb_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr);
      
      cb_logger_facade(const cb_logger_facade& other) = delete;
      cb_logger_facade& operator=(const cb_logger_facade& other) = delete;
      cb_logger_facade(cb_logger_facade&& other) = delete;
      cb_logger_facade& operator=(cb_logger_facade&& other) = delete;

      ~cb_logger_facade() = default;

      int init(api_status* status);

      int log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode = ONLINE);
    
    private:
      const int version;
      const std::unique_ptr<interaction_logger> v1;
    };

    class ccb_logger_facade {
    public:
      ccb_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr);

      ccb_logger_facade(const ccb_logger_facade& other) = delete;
      ccb_logger_facade& operator=(const ccb_logger_facade& other) = delete;
      ccb_logger_facade(ccb_logger_facade&& other) = delete;
      ccb_logger_facade& operator=(ccb_logger_facade&& other) = delete;

      ~ccb_logger_facade() = default;

      int init(api_status* status);

      int log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);

    private:
      const int version;
      const std::unique_ptr<ccb_logger> v1;
    };

    class slates_logger_facade {
      slates_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr);

      slates_logger_facade(const slates_logger_facade& other) = delete;
      slates_logger_facade& operator=(const slates_logger_facade& other) = delete;
      slates_logger_facade(slates_logger_facade&& other) = delete;
      slates_logger_facade& operator=(slates_logger_facade&& other) = delete;

      ~slates_logger_facade() = default;

      int init(api_status* status);

      int log_decision(const std::string& event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);

    private:
      const int version;
      const std::unique_ptr<slates_logger> v1;
    };

    class outcome_logger_facade {
    public:
      outcome_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr);

      outcome_logger_facade(const outcome_logger_facade& other) = delete;
      outcome_logger_facade& operator=(const outcome_logger_facade& other) = delete;
      outcome_logger_facade(outcome_logger_facade&& other) = delete;
      outcome_logger_facade& operator=(outcome_logger_facade&& other) = delete;

      ~outcome_logger_facade() = default;

      int init(api_status* status);

      int log(const char* event_id, float outcome, api_status* status);

      int log(const char* event_id, const char* outcome, api_status* status);

      int report_action_taken(const char* event_id, api_status* status);

    private:
      const int version;
      const std::unique_ptr<observation_logger> v1;
    };
  }
}