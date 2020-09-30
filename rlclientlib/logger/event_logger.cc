#include "event_logger.h"
#include "ranking_event.h"
#include "err_constants.h"
#include "time_helper.h"
namespace reinforcement_learning { namespace logger {
  content_encoding_enum to_content_encoding_enum(const char* content_encoding)
  {
    if (std::strcmp(content_encoding, value::CONTENT_ENCODING_ZSTD_AND_DEDUP) == 0)
    {
      return content_encoding_enum::ZSTD_AND_DEDUP;
    }
    else
    {
      return content_encoding_enum::IDENTITY;
    }
  }

  int interaction_logger::log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(ranking_event::choose_rank(event_id, context, flags, response, now, 1.0f, learning_mode), status);
  }

  int ccb_logger::log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
    const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(std::move(decision_ranking_event::request_decision(event_ids, context, flags, action_ids, pdfs, model_version, now)), status);
  }
  int multi_slot_logger::log_decision(const std::string &event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {

    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(std::move(multi_slot_decision_event::request_decision(event_id, context, flags, action_ids, pdfs, model_version, now)), status);
  }

  int observation_logger::report_action_taken(const char* event_id, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(outcome_event::report_action_taken(event_id, now), status);
  }

  int generic_event_logger::log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(generic_event(event_id, now, type, std::move(payload)), status);
  }
}}
