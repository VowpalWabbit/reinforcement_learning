#include "logger_facade.h"
#include "err_constants.h"

namespace err = reinforcement_learning::error_code;

namespace reinforcement_learning {
  using model_type = reinforcement_learning::model_management::model_type_t;

  namespace logger {
    int protocol_not_supported(api_status* status) {
      RETURN_ERROR_ARG(nullptr, status, protocol_not_supported, "Current protocol version is not supported");
    }

    interaction_logger_facade::interaction_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : _version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , _v1_cb(_version == 1 ? new interaction_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr)
    , _v2(_version == 2 ? new generic_event_logger(
      sender,
      c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
      c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
      c.get_int(name::INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
      c.get(name::QUEUE_MODE, "DROP"),
      watchdog,
      time_provider,
      perror_cb) : nullptr) {
    }

    int interaction_logger_facade::init(api_status* status) {
      switch (_version) {
        case 1: return _v1_cb->init(status);
        case 2: return _v2->init(status);
        default: return protocol_not_supported(status);
      }
    }

    int interaction_logger_facade::log(const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode) {
      switch (_version) {
        case 1: return _v1_cb->log(response.get_event_id(), context, flags, response, status, learning_mode);
        case 2:
          v2::LearningModeType lmt;
          RETURN_IF_FAIL(get_learning_mode(learning_mode, lmt, status));
          return _v2->log(response.get_event_id(), _serializer.event(context, flags, lmt, response), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

  ca_logger_facade::ca_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : _version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , _v2(_version == 2 ? new generic_event_logger(
      sender,
      c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
      c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
      c.get_int(name::INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
      c.get(name::QUEUE_MODE, "DROP"),
      watchdog,
      time_provider,
      perror_cb) : nullptr) {}

    int ca_logger_facade::init(api_status* status) {
      switch (_version) {
        case 1: return err::success;
        case 2: return _v2->init(status);
        default: return protocol_not_supported(status);
      }
    }

    int ca_logger_facade::log_continuous_action(const char* context, unsigned int flags, const continuous_action_response& response, api_status* status) {
      switch (_version) {
        case 2: return _v2->log(response.get_event_id(), _serializer.event(context, flags, response), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    ccb_logger_facade::ccb_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : _version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , _v1(_version == 1 ? new ccb_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr) {
    }

    int ccb_logger_facade::init(api_status* status) {
      switch (_version) {
        case 1: return _v1->init(status);
        case 2: return error_code::success;
        default: return protocol_not_supported(status);
      }
    }

    int ccb_logger_facade::log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      switch (_version) {
        case 1: return _v1->log_decisions(event_ids, context, flags, action_ids, pdfs, model_version, status);
        default: return protocol_not_supported(status);
      }
    }

    multi_slot_logger_facade::multi_slot_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : _version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , _v1(_version == 1 ? new slates_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr)
    , _v2(_version == 2 ? new generic_event_logger(
      sender,
      c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
      c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
      c.get_int(name::INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
      c.get(name::QUEUE_MODE, "DROP"),
      watchdog,
      time_provider,
      perror_cb) : nullptr) {
    }

    int multi_slot_logger_facade::init(api_status* status) {
      switch (_version) {
        case 1: return _v1->init(status);
        case 2: return _v2->init(status);
        default: return protocol_not_supported(status);
      }
    }

    int multi_slot_model_type_to_payload_type(model_type model_type, generic_event::payload_type_t& payload_type, api_status* status)
    {
      //XXX out params must be always initialized. This is an ok default
      payload_type = generic_event::payload_type_t::PayloadType_Slates;
      switch(model_type) {
        case reinforcement_learning::model_management::model_type_t::CCB: payload_type = generic_event::payload_type_t::PayloadType_CCB; break;
        case reinforcement_learning::model_management::model_type_t::SLATES: payload_type = generic_event::payload_type_t::PayloadType_Slates; break;
        default: RETURN_ERROR_ARG(nullptr, status, invalid_argument, "Invalid model_type, only Slates and CCB are supported with multi_slot decisions");
      }
      return error_code::success;
    }

    int multi_slot_logger_facade::log_decision(model_type model_type, const std::string& event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      switch (_version) {
        case 1: {
          if (model_type != reinforcement_learning::model_management::model_type_t::SLATES)
            RETURN_ERROR_ARG(nullptr, status, invalid_argument, "multi_slot logger under v1 protocol can only log slates.");
          return _v1->log_decision(event_id, context, flags, action_ids, pdfs, model_version, status);
        }
        case 2: {
          generic_event::payload_type_t payload_type;
          RETURN_IF_FAIL(multi_slot_model_type_to_payload_type(model_type, payload_type, status));
          return _v2->log(event_id.c_str(), _serializer.event(context, flags, action_ids, pdfs, model_version), payload_type, status);
        }
        default: return protocol_not_supported(status);
      }
    }

    observation_logger_facade::observation_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : _version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , _v1(_version == 1 ? new observation_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr)
    , _v2(_version == 2 ? new generic_event_logger(
      sender,
      c.get_int(name::OBSERVATION_SEND_HIGH_WATER_MARK, 198 * 1024),
      c.get_int(name::OBSERVATION_SEND_BATCH_INTERVAL_MS, 1000),
      c.get_int(name::OBSERVATION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
      c.get(name::QUEUE_MODE, "DROP"),
      watchdog,
      time_provider,
      perror_cb) : nullptr) {
    }

    int observation_logger_facade::init(api_status* status) {
      switch (_version) {
        case 1: return _v1->init(status);
        case 2: return _v2->init(status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* event_id, float outcome, api_status* status) {
      switch (_version) {
        case 1: return _v1->log(event_id, outcome, status);
        case 2: return _v2->log(event_id, _serializer.numeric_event(outcome), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* event_id, const char* outcome, api_status* status) {
      switch (_version) {
        case 1: return _v1->log(event_id, outcome, status);
        case 2: return _v2->log(event_id, _serializer.string_event(outcome), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }


    int observation_logger_facade::log(const char* primary_id, int secondary_id, float outcome, api_status* status) {
      switch (_version) {
        case 2: return _v2->log(primary_id, _serializer.numeric_event(secondary_id, outcome), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* primary_id, int secondary_id, const char* outcome, api_status* status) {
      switch (_version) {
        case 2: return _v2->log(primary_id, _serializer.string_event(secondary_id, outcome), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* primary_id, const char* secondary_id, float outcome, api_status* status) {
      switch (_version) {
        case 2: return _v2->log(primary_id, _serializer.numeric_event(secondary_id, outcome), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* primary_id, const char* secondary_id, const char* outcome, api_status* status) {
      switch (_version) {
        case 2: return _v2->log(primary_id, _serializer.string_event(secondary_id, outcome), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::report_action_taken(const char* event_id, api_status* status) {
      switch (_version) {
        case 1: return _v1->report_action_taken(event_id, status);
        case 2: return _v2->log(event_id, _serializer.report_action_taken(), _serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }
  }
}
