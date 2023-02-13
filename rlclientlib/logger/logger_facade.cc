#include "logger_facade.h"

#include "err_constants.h"

namespace err = reinforcement_learning::error_code;

namespace reinforcement_learning
{
using model_type_t = reinforcement_learning::model_management::model_type_t;

namespace logger
{
int protocol_not_supported(api_status* status)
{
  RETURN_ERROR_ARG(nullptr, status, protocol_not_supported, "Current protocol version is not supported");
}

template <typename T>
i_async_batcher<T>* create_legacy_async_batcher(const utility::configuration& c, i_message_sender* sender,
    utility::watchdog& watchdog, error_callback_fn* perror_cb, const char* section,
    typename async_batcher<T, fb_collection_serializer>::shared_state_t& shared_state)
{
  auto config = utility::get_batcher_config(c, section);
  return new async_batcher<T, fb_collection_serializer>(sender, watchdog, shared_state, perror_cb, config);
}

interaction_logger_facade::interaction_logger_facade(model_type_t model_type, const utility::configuration& c,
    i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, i_logger_extensions* ext,
    error_callback_fn* perror_cb)
    : _model_type(model_type)
    , _version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , _serializer_shared_state(0)
    , _ext_p(ext)
    , _v1_cb(_version == 1 && _model_type == model_type_t::CB
              ? new interaction_logger(time_provider,
                    create_legacy_async_batcher<ranking_event>(
                        c, sender, watchdog, perror_cb, INTERACTION_SECTION, _serializer_shared_state))
              : nullptr)
    , _v1_ccb(_version == 1 && _model_type == model_type_t::CCB
              ? new ccb_logger(time_provider,
                    create_legacy_async_batcher<decision_ranking_event>(
                        c, sender, watchdog, perror_cb, INTERACTION_SECTION, _serializer_shared_state))
              : nullptr)
    , _v1_multislot(_version == 1 && _model_type == model_type_t::SLATES
              ? new multi_slot_logger(time_provider,
                    create_legacy_async_batcher<multi_slot_decision_event>(
                        c, sender, watchdog, perror_cb, INTERACTION_SECTION, _serializer_shared_state))
              : nullptr)
    , _v2(_version == 2
              ? new generic_event_logger(time_provider,
                    ext->create_batcher(sender, watchdog, perror_cb, INTERACTION_SECTION), c.get(name::APP_ID, ""))
              : nullptr)
{
}

int interaction_logger_facade::init(api_status* status)
{
  switch (_version)
  {
    case 1:
      switch (_model_type)
      {
        case model_type_t::CB:
          return _v1_cb->init(status);
        case model_type_t::CCB:
          return _v1_ccb->init(status);
        case model_type_t::SLATES:
          return _v1_multislot->init(status);
        default:
          return protocol_not_supported(status);
      }
    case 2:
      return _v2->init(status);
    default:
      return protocol_not_supported(status);
  }
}

int interaction_logger_facade::log(string_view context, unsigned int flags, const ranking_response& response,
    api_status* status, learning_mode learning_mode)
{
  switch (_version)
  {
    case 1:
      return _v1_cb->log(response.get_event_id(), context, flags, response, status, learning_mode);
    case 2:
    {
      v2::LearningModeType lmt;
      RETURN_IF_FAIL(get_learning_mode(learning_mode, lmt, status));

      // needed for the serializer
      std::vector<uint64_t> action_ids;
      std::vector<float> probabilities;
      std::string model_id(response.get_model_id());
      for (auto const& r : response)
      {
        action_ids.push_back(r.action_id + 1);
        probabilities.push_back(r.probability);
      }

      return _v2->log(response.get_event_id(), context, _serializer_cb.type, _ext_p, _serializer_cb, status, flags, lmt,
          action_ids, probabilities, model_id);
    }
    default:
      return protocol_not_supported(status);
  }
}

int interaction_logger_facade::log(const char* episode_id, const char* previous_id, string_view context,
    unsigned int flags, const ranking_response& response, api_status* status)
{
  switch (_version)
  {
    case 2:
    {
      // needed for the serializer
      std::vector<uint64_t> action_ids;
      std::vector<float> probabilities;
      for (auto const& r : response)
      {
        action_ids.push_back(r.action_id + 1);
        probabilities.push_back(r.probability);
      }

      std::string event_id(response.get_event_id());
      std::string model_id(response.get_model_id());
      std::string previous_id_str(previous_id ? previous_id : "");

      return _v2->log(episode_id, context, _multistep_serializer.type, _ext_p, _multistep_serializer, status,
          previous_id_str, flags, action_ids, probabilities, event_id, model_id);
    }
    default:
      return protocol_not_supported(status);
  }
}

int interaction_logger_facade::log_decisions(std::vector<const char*>& event_ids, string_view context,
    unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
    const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status)
{
  switch (_version)
  {
    case 1:
      return _v1_ccb->log_decisions(event_ids, context, flags, action_ids, pdfs, model_version, status);
    default:
      return protocol_not_supported(status);
  }
}

int multi_slot_model_type_to_payload_type(
    model_type_t model_type, generic_event::payload_type_t& payload_type, api_status* status)
{
  // XXX out params must be always initialized. This is an ok default
  payload_type = generic_event::payload_type_t::PayloadType_Slates;
  switch (model_type)
  {
    case model_type_t::CCB:
      payload_type = generic_event::payload_type_t::PayloadType_CCB;
      break;
    case model_type_t::SLATES:
      payload_type = generic_event::payload_type_t::PayloadType_Slates;
      break;
    default:
      RETURN_ERROR_ARG(nullptr, status, invalid_argument,
          "Invalid model_type, only Slates and CCB are supported with multi_slot decisions");
  }
  return error_code::success;
}

int interaction_logger_facade::log_decision(const std::string& event_id, string_view context, unsigned int flags,
    const std::vector<std::vector<uint32_t>>& action_ids, const std::vector<std::vector<float>>& pdfs,
    const std::string& model_version, const std::vector<std::string>& slot_ids, api_status* status,
    const std::vector<int>& baseline_actions, learning_mode learning_mode)
{
  switch (_version)
  {
    case 1:
    {
      switch (_model_type)
      {
        case model_type_t::SLATES:
          return _v1_multislot->log_decision(event_id, context, flags, action_ids, pdfs, model_version, status);
        default:
          RETURN_ERROR_ARG(
              nullptr, status, protocol_not_supported, "multi_slot logger under v1 protocol can only log slates.");
      }
    }
    case 2:
    {
      v2::LearningModeType lmt;
      RETURN_IF_FAIL(get_learning_mode(learning_mode, lmt, status));

      generic_event::payload_type_t payload_type;
      RETURN_IF_FAIL(multi_slot_model_type_to_payload_type(_model_type, payload_type, status));

      return _v2->log(event_id.c_str(), context, payload_type, _ext_p, _serializer_multislot, status, flags, action_ids,
          pdfs, model_version, slot_ids, baseline_actions, lmt);
    }
    default:
      return protocol_not_supported(status);
  }
}

int interaction_logger_facade::log_continuous_action(
    string_view context, unsigned int flags, const continuous_action_response& response, api_status* status)
{
  switch (_version)
  {
    case 2:
    {
      // Create a string out of char* returned by get_model_id()
      // so that it can be copied by value and persist after char* goes out of scope
      auto model_id = std::string(response.get_model_id());
      return _v2->log(response.get_event_id(), context, _serializer_ca.type, _ext_p, _serializer_ca, status, flags,
          response.get_chosen_action(), response.get_chosen_action_pdf_value(), model_id);
    }
    default:
      return protocol_not_supported(status);
  }
}

observation_logger_facade::observation_logger_facade(const utility::configuration& c, i_message_sender* sender,
    utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : _version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , _serializer_shared_state(0)
    , _v1(_version == 1 ? new observation_logger(time_provider,
                              create_legacy_async_batcher<outcome_event>(
                                  c, sender, watchdog, perror_cb, OBSERVATION_SECTION, _serializer_shared_state))
                        : nullptr)
    , _v2(_version == 2 ? new generic_event_logger(time_provider,
                              create_legacy_async_batcher<generic_event>(
                                  c, sender, watchdog, perror_cb, OBSERVATION_SECTION, _serializer_shared_state),
                              c.get(name::APP_ID, ""))
                        : nullptr)
{
}

int observation_logger_facade::init(api_status* status)
{
  switch (_version)
  {
    case 1:
      return _v1->init(status);
    case 2:
      return _v2->init(status);
    default:
      return protocol_not_supported(status);
  }
}

int observation_logger_facade::log(const char* event_id, float outcome, api_status* status)
{
  switch (_version)
  {
    case 1:
      return _v1->log(event_id, outcome, status);
    case 2:
      return _v2->log(event_id, reinforcement_learning::logger::outcome_serializer::numeric_event(outcome),
          _serializer.type, event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}

int observation_logger_facade::log(const char* event_id, const char* outcome, api_status* status)
{
  switch (_version)
  {
    case 1:
      return _v1->log(event_id, outcome, status);
    case 2:
      return _v2->log(event_id, reinforcement_learning::logger::outcome_serializer::string_event(outcome),
          _serializer.type, event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}

int observation_logger_facade::log(const char* primary_id, int secondary_id, float outcome, api_status* status)
{
  switch (_version)
  {
    case 2:
      return _v2->log(primary_id,
          reinforcement_learning::logger::outcome_serializer::numeric_event(secondary_id, outcome), _serializer.type,
          event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}

int observation_logger_facade::log(const char* primary_id, int secondary_id, const char* outcome, api_status* status)
{
  switch (_version)
  {
    case 2:
      return _v2->log(primary_id,
          reinforcement_learning::logger::outcome_serializer::string_event(secondary_id, outcome), _serializer.type,
          event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}

int observation_logger_facade::log(const char* primary_id, const char* secondary_id, float outcome, api_status* status)
{
  switch (_version)
  {
    case 2:
      return _v2->log(primary_id,
          reinforcement_learning::logger::outcome_serializer::numeric_event(secondary_id, outcome), _serializer.type,
          event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}

int observation_logger_facade::log(
    const char* primary_id, const char* secondary_id, const char* outcome, api_status* status)
{
  switch (_version)
  {
    case 2:
      return _v2->log(primary_id,
          reinforcement_learning::logger::outcome_serializer::string_event(secondary_id, outcome), _serializer.type,
          event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}

int observation_logger_facade::report_action_taken(const char* event_id, api_status* status)
{
  switch (_version)
  {
    case 1:
      return _v1->report_action_taken(event_id, status);
    case 2:
      return _v2->log(event_id, reinforcement_learning::logger::outcome_serializer::report_action_taken(),
          _serializer.type, event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}

int observation_logger_facade::report_action_taken(const char* primary_id, const char* secondary_id, api_status* status)
{
  switch (_version)
  {
    case 2:
      return _v2->log(primary_id, reinforcement_learning::logger::outcome_serializer::report_action_taken(secondary_id),
          _serializer.type, event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}

// TODO: Do we need an EPISODE_SECTION for the config? Just use OBSERVATION_SECTION for now
episode_logger_facade::episode_logger_facade(const utility::configuration& c, i_message_sender* sender,
    utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb)
    : _version(c.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
    , _serializer_shared_state(0)
    , _v2(_version == 2 ? new generic_event_logger(time_provider,
                              create_legacy_async_batcher<generic_event>(
                                  c, sender, watchdog, perror_cb, OBSERVATION_SECTION, _serializer_shared_state),
                              c.get(name::APP_ID, ""))
                        : nullptr)
{
}

int episode_logger_facade::init(api_status* status)
{
  switch (_version)
  {
    case 2:
      return _v2->init(status);
    default:
      return protocol_not_supported(status);
  }
}

int episode_logger_facade::log(const char* episode_id, api_status* status)
{
  switch (_version)
  {
    case 2:
      return _v2->log(episode_id, reinforcement_learning::logger::episode_serializer::episode_event(episode_id),
          _episode_serializer.type, event_content_type::IDENTITY, status);
    default:
      return protocol_not_supported(status);
  }
}
}  // namespace logger
}  // namespace reinforcement_learning
