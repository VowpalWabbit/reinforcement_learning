#pragma once

#include "api_status.h"
#include "configuration.h"
#include "constants.h"
#include "learning_mode.h"
#include "ranking_response.h"
#include "error_callback_fn.h"
#include "utility/watchdog.h"

#include "message_sender.h"
#include "time_helper.h"

#include "event_logger.h"
#include "model_mgmt.h"

#include "trace_logger.h"

#include "serialization/payload_serializer.h"

namespace reinforcement_learning
{
  namespace logger {
    class i_logger_extensions {
    protected:
      const utility::configuration& _config;
    public:
      i_logger_extensions(const utility::configuration&);

      virtual ~i_logger_extensions();

      virtual bool is_object_extraction_enabled() const = 0;
      virtual bool is_serialization_transform_enabled() const = 0;

      virtual i_async_batcher<generic_event>* create_batcher(i_message_sender* sender, utility::watchdog& watchdog, error_callback_fn* perror_cb, i_trace* trace, const char* section) = 0;
      virtual int transform_payload_and_extract_objects(const char* context, std::string& edited_payload, generic_event::object_list_t& objects, api_status* status) = 0;
      virtual int transform_serialized_payload(generic_event::payload_buffer_t& input, event_content_type &content_type, api_status* status) const = 0;

      static i_logger_extensions* get_extensions(const utility::configuration& config, i_time_provider* time_provider);
    };

    class interaction_logger_facade {
    public:
      interaction_logger_facade(reinforcement_learning::model_management::model_type_t model_type,
        const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog,
        i_time_provider* time_provider, i_logger_extensions& ext, i_trace* trace, error_callback_fn* perror_cb = nullptr);

      interaction_logger_facade(const interaction_logger_facade& other) = delete;
      interaction_logger_facade& operator=(const interaction_logger_facade& other) = delete;
      interaction_logger_facade(interaction_logger_facade&& other) = delete;
      interaction_logger_facade& operator=(interaction_logger_facade&& other) = delete;

      ~interaction_logger_facade() = default;

      int init(api_status* status);
	  void flush();

      //CB v1/v2
      int log(const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode = ONLINE);

      //CCB v1
      int log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);

      //Multislot (Slates v1/v2 + CCB v2)
      int log_decision(const std::string& event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
        const std::vector<std::vector<float>>& pdfs, const std::string& model_version, const std::vector<std::string>& slot_ids, api_status* status, const std::vector<int>& baseline_actions, learning_mode learning_mode = ONLINE);

      //Continuous
      int log_continuous_action(const char* context, unsigned int flags, const continuous_action_response& response, api_status* status);

    private:
      const reinforcement_learning::model_management::model_type_t _model_type;
      const int _version;
      int _serializer_shared_state;
      i_logger_extensions& _ext;

      const std::unique_ptr<interaction_logger> _v1_cb;
      const std::unique_ptr<ccb_logger> _v1_ccb;
      const std::unique_ptr<multi_slot_logger> _v1_multislot;

      const std::unique_ptr<generic_event_logger> _v2;

      const cb_serializer _serializer_cb;
      const multi_slot_serializer _serializer_multislot;
      const ca_serializer _serializer_ca;

    };

    class observation_logger_facade {
    public:
      observation_logger_facade(const utility::configuration& c,
        i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, i_trace* trace, error_callback_fn* perror_cb = nullptr);

      observation_logger_facade(const observation_logger_facade& other) = delete;
      observation_logger_facade& operator=(const observation_logger_facade& other) = delete;
      observation_logger_facade(observation_logger_facade&& other) = delete;
      observation_logger_facade& operator=(observation_logger_facade&& other) = delete;

      ~observation_logger_facade() = default;

      int init(api_status* status);
	  void flush();


      int log(const char* event_id, float outcome, api_status* status);
      int log(const char* event_id, const char* outcome, api_status* status);

      int log(const char* event_id, int index, float outcome, api_status* status);
      int log(const char* event_id, int index, const char* outcome, api_status* status);
      int log(const char* event_id, const char* index, float outcome, api_status* status);
      int log(const char* event_id, const char* index, const char* outcome, api_status* status);

      int report_action_taken(const char* event_id, api_status* status);

    private:
      const int _version;
      int _serializer_shared_state;
      const std::unique_ptr<observation_logger> _v1;
      const std::unique_ptr<generic_event_logger> _v2;
      const outcome_serializer _serializer;
    };
  }
}
