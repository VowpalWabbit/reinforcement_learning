#include "event_logger.h"
#include "ranking_event.h"
#include "err_constants.h"
#include "time_helper.h"
namespace reinforcement_learning { namespace logger {
  int interaction_logger::log(string_view event_id, string_view context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(ranking_event::choose_rank(event_id, context, flags, response, now, 1.0f, learning_mode), status);
  }

  int ccb_logger::log_decisions(std::vector<string_view>& event_ids, string_view context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
    const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(std::move(decision_ranking_event::request_decision(event_ids, context, flags, action_ids, pdfs, model_version, now)), status);
  }

  int multi_slot_logger::log_decision(string_view event_id, string_view context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {

    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(std::move(multi_slot_decision_event::request_decision(event_id, context, flags, action_ids, pdfs, model_version, now)), status);
  }

  int observation_logger::report_action_taken(string_view event_id, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(outcome_event::report_action_taken(event_id, now), status);
  }

  int generic_event_logger::log(string_view event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, event_content_type content_type, api_status* status) {
    generic_event::object_list_t objects;
    return log(event_id, std::move(payload), type, content_type, std::move(objects), status);
  }

  int generic_event_logger::log(string_view event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, event_content_type content_type, generic_event::object_list_t&& objects, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    return append(generic_event(event_id, now, type, std::move(payload), content_type, std::move(objects), _app_id), status);
  }

}}
