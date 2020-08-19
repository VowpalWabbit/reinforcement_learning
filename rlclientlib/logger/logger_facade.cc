#include "logger_facade.h"
#include "err_constants.h"

namespace reinforcement_learning {
  namespace logger {
    int protocol_not_supported(api_status* status) {
      api_status::try_update(status, error_code::protocol_not_supported,
        "Current protocol version is not supported");
      return error_code::protocol_not_supported;
    }

    cb_logger_facade::cb_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , v1(version == 1 ? new interaction_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr)
    , v2(version == 2 ? new generic_event_logger(
      sender,
      c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
      c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
      c.get_int(name::INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
      c.get(name::QUEUE_MODE, "DROP"),
      watchdog,
      time_provider,
      perror_cb) : nullptr) {
    }

    int cb_logger_facade::init(api_status* status) {
      switch (version) {
        case 1: return v1->init(status);
        case 2: return v2->init(status);
        default: return protocol_not_supported(status);
      }
    }

    int cb_logger_facade::log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode) {
      switch (version) {
        case 1: return v1->log(event_id, context, flags, response, status, learning_mode);
        case 2: return v2->log(event_id, serializer.event(context, flags, learning_mode, response), serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    ccb_logger_facade::ccb_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , v1(version == 1 ? new ccb_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr)
    , v2(version == 2 ? new generic_event_logger(
      sender,
      c.get_int(name::DECISION_SEND_HIGH_WATER_MARK, 198 * 1024),
      c.get_int(name::DECISION_SEND_BATCH_INTERVAL_MS, 1000),
      c.get_int(name::DECISION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
      c.get(name::QUEUE_MODE, "DROP"),
      watchdog,
      time_provider,
      perror_cb) : nullptr) {
    }

    int ccb_logger_facade::init(api_status* status) {
      switch (version) {
        case 1: return v1->init(status);
        case 2: return v2->init(status);
        default: return protocol_not_supported(status);
      }
    }

    int ccb_logger_facade::log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      switch (version) {
        case 1: return v1->log_decisions(event_ids, context, flags, action_ids, pdfs, model_version, status);
        default: return protocol_not_supported(status);
      }
    }

    int ccb_logger_facade::log_decisions(const char* event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      switch (version) {
        case 2: return v2->log(event_id, serializer.event(context, flags, action_ids, pdfs, model_version), serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    slates_logger_facade::slates_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , v1(version == 1 ? new slates_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr)
    , v2(version == 2 ? new generic_event_logger(
      sender,
      c.get_int(name::DECISION_SEND_HIGH_WATER_MARK, 198 * 1024),
      c.get_int(name::DECISION_SEND_BATCH_INTERVAL_MS, 1000),
      c.get_int(name::DECISION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
      c.get(name::QUEUE_MODE, "DROP"),
      watchdog,
      time_provider,
      perror_cb) : nullptr) {
    }

    int slates_logger_facade::init(api_status* status) {
      switch (version) {
        case 1: return v1->init(status);
        case 2: return v2->init(status);
        default: return protocol_not_supported(status);
      }
    }

    int slates_logger_facade::log_decision(const std::string& event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
      switch (version) {
        case 1: return v1->log_decision(event_id, context, flags, action_ids, pdfs, model_version, status);
        case 2: return v2->log(event_id.c_str(), serializer.event(context, flags, action_ids, pdfs, model_version), serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    observation_logger_facade::observation_logger_facade(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , v1(version == 1 ? new observation_logger(c, sender, watchdog, time_provider, perror_cb) : nullptr)
    , v2(version == 2 ? new generic_event_logger(
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
      switch (version) {
        case 1: return v1->init(status);
        case 2: return v2->init(status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* event_id, float outcome, api_status* status) {
      switch (version) {
        case 1: return v1->log(event_id, outcome, status);
        case 2: return v2->log(event_id, serializer.event(outcome), serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* event_id, const char* outcome, api_status* status) {
      switch (version) {
        case 1: return v1->log(event_id, outcome, status);
        case 2: return v2->log(event_id, serializer.event(outcome), serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* event_id, int index, float outcome, api_status* status) {
      switch (version) {
      case 1: return protocol_not_supported(status);
      case 2: return v2->log(event_id, serializer.event(index, outcome), serializer.type, status);
      default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::log(const char* event_id, int index, const char* outcome, api_status* status) {
      switch (version) {
      case 1: return protocol_not_supported(status);
      case 2: return v2->log(event_id, serializer.event(index, outcome), serializer.type, status);
      default: return protocol_not_supported(status);
      }
    }

    int observation_logger_facade::report_action_taken(const char* event_id, api_status* status) {
      switch (version) {
        case 1: return v1->report_action_taken(event_id, status);
        case 2: return v2->log(event_id, serializer.report_action_taken(), serializer.type, status);
        default: return protocol_not_supported(status);
      }
    }
  }
}
