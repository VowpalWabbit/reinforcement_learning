#pragma once

#include "api_status.h"
#include "configuration.h"
#include "constants.h"
#include "error_callback_fn.h"
#include "event_logger.h"
#include "learning_mode.h"
#include "logger/logger_extensions.h"
#include "message_sender.h"
#include "model_mgmt.h"
#include "ranking_response.h"
#include "serialization/payload_serializer.h"
#include "time_helper.h"
#include "utility/watchdog.h"

#include <functional>
#include <memory>

namespace reinforcement_learning
{
namespace logger
{
class interaction_logger_facade
{
public:
  interaction_logger_facade(reinforcement_learning::model_management::model_type_t model_type,
      const utility::configuration& c, std::unique_ptr<i_message_sender> sender, utility::watchdog& watchdog,
      std::unique_ptr<i_time_provider> time_provider, i_logger_extensions* ext, error_callback_fn* perror_cb = nullptr);

  interaction_logger_facade(const interaction_logger_facade& other) = delete;
  interaction_logger_facade& operator=(const interaction_logger_facade& other) = delete;
  interaction_logger_facade(interaction_logger_facade&& other) = delete;
  interaction_logger_facade& operator=(interaction_logger_facade&& other) = delete;

  ~interaction_logger_facade() = default;

  int init(api_status* status);

  // CB v1/v2
  int log(string_view context, unsigned int flags, const ranking_response& response, api_status* status,
      learning_mode learning_mode = ONLINE);

  int log_decisions(std::vector<const char*>& event_ids, string_view context, unsigned int flags,
      const std::vector<std::vector<uint32_t>>& action_ids, const std::vector<std::vector<float>>& pdfs,
      const std::string& model_version, api_status* status);

  // Multislot (Slates v1/v2 + CCB v2)
  int log_decision(const std::string& event_id, string_view context, unsigned int flags,
      const std::vector<std::vector<uint32_t>>& action_ids, const std::vector<std::vector<float>>& pdfs,
      const std::string& model_version, const std::vector<std::string>& slot_ids, api_status* status,
      const std::vector<int>& baseline_actions, learning_mode learning_mode = ONLINE);

  // Continuous
  int log_continuous_action(
      string_view context, unsigned int flags, const continuous_action_response& response, api_status* status);

  // Multistep
  int log(const char* episode_id, const char* previous_id, string_view context, unsigned int flags,
      const ranking_response& response, api_status* status);

private:
  const reinforcement_learning::model_management::model_type_t _model_type;
  const int _version;
  int _serializer_shared_state;
  // _ext_p is owned by live_model_impl
  i_logger_extensions* _ext_p;

  const std::unique_ptr<interaction_logger> _v1_cb;
  const std::unique_ptr<ccb_logger> _v1_ccb;
  const std::unique_ptr<multi_slot_logger> _v1_multislot;

  const std::unique_ptr<generic_event_logger> _v2;

  const cb_serializer _serializer_cb;
  const multi_slot_serializer _serializer_multislot;
  const ca_serializer _serializer_ca;
  const multistep_serializer _multistep_serializer;
};

class observation_logger_facade
{
public:
  observation_logger_facade(const utility::configuration& c, std::unique_ptr<i_message_sender> sender,
      utility::watchdog& watchdog, std::unique_ptr<i_time_provider> time_provider,
      error_callback_fn* perror_cb = nullptr);

  observation_logger_facade(const observation_logger_facade& other) = delete;
  observation_logger_facade& operator=(const observation_logger_facade& other) = delete;
  observation_logger_facade(observation_logger_facade&& other) = delete;
  observation_logger_facade& operator=(observation_logger_facade&& other) = delete;

  ~observation_logger_facade() = default;

  int init(api_status* status);

  int log(const char* event_id, float outcome, api_status* status);
  int log(const char* event_id, const char* outcome, api_status* status);

  int log(const char* primary_id, int secondary_id, float outcome, api_status* status);
  int log(const char* primary_id, int secondary_id, const char* outcome, api_status* status);
  int log(const char* primary_id, const char* secondary_id, float outcome, api_status* status);
  int log(const char* primary_id, const char* secondary_id, const char* outcome, api_status* status);

  int report_action_taken(const char* event_id, api_status* status);
  int report_action_taken(const char* primary_id, const char* secondary_id, api_status* status);

private:
  const int _version;
  int _serializer_shared_state;
  const std::unique_ptr<observation_logger> _v1;
  const std::unique_ptr<generic_event_logger> _v2;
  const outcome_serializer _serializer;
};

class episode_logger_facade
{
public:
  episode_logger_facade(const utility::configuration& c, std::unique_ptr<i_message_sender> sender,
      utility::watchdog& watchdog, std::unique_ptr<i_time_provider> time_provider,
      error_callback_fn* perror_cb = nullptr);

  episode_logger_facade(const episode_logger_facade& other) = delete;
  episode_logger_facade& operator=(const episode_logger_facade& other) = delete;
  episode_logger_facade(episode_logger_facade&& other) = delete;
  episode_logger_facade& operator=(episode_logger_facade&& other) = delete;

  ~episode_logger_facade() = default;

  int init(api_status* status);

  int log(const char* episode_id, api_status* status);

private:
  const int _version;
  int _serializer_shared_state;
  const std::unique_ptr<generic_event_logger> _v2;
  const episode_serializer _episode_serializer;
};
}  // namespace logger
}  // namespace reinforcement_learning
