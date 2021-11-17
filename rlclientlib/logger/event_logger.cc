#include "event_logger.h"
#include "ranking_event.h"
#include "err_constants.h"
#include "time_helper.h"

#include <functional>
namespace reinforcement_learning { namespace logger {
using namespace std::placeholders;
  int interaction_logger::log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    // std::bind being used here to avoid needing to copy the actual event. Lambdas allow capture statements
    // in C++14, so we can remove the bind usage at that point
    auto evt_fn = std::bind(
      [](ranking_event& out_evt, api_status* status, ranking_event in_evt)->int {
        out_evt = std::move(in_evt);
        return error_code::success;
      },
      _1,
      _2,
      ranking_event::choose_rank(event_id, context, flags, response, now, 1.0f, learning_mode)
    );
    return append(std::move(evt_fn), event_id, 1 /*TODO: fix size estimate*/, status);
    
  }

  int ccb_logger::log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
    const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    auto evt = decision_ranking_event::request_decision(event_ids, context, flags, action_ids, pdfs, model_version, now);
    // Short string optimization makes this unreliable. For now, just use event_ids[0] which the event uses underneath
    // const char* evt_id = evt.get_seed_id().c_str();
    const char* evt_id = event_ids[0];
    /*
    // TODO: For some reason, the bind function is not properly binding to the std::function.
    // This means we need to do a full copy of the event
    std::function<int(decision_ranking_event&, api_status*)> evt_fn = std::bind(
      [](decision_ranking_event& out_evt, api_status* status, decision_ranking_event in_evt)->int {
        out_evt = std::move(in_evt);
        return error_code::success;
      },
      _1,
      _2,
      std::move(evt)
    );*/
    auto evt_fn = [evt](decision_ranking_event& out_evt, api_status* status)->int {
      out_evt = std::move(evt);
      return error_code::success;
    };
    return append(std::move(evt_fn), evt_id, 1, status);
  }

  int multi_slot_logger::log_decision(const std::string &event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {

    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    
    auto evt_fn = std::bind(
      [](multi_slot_decision_event& out_evt, api_status* status, multi_slot_decision_event in_evt)->int {
        out_evt = std::move(in_evt);
        return error_code::success;
      },
      _1,
      _2,
      multi_slot_decision_event::request_decision(event_id, context, flags, action_ids, pdfs, model_version, now)
    );
    return append(std::move(evt_fn), event_id.c_str(), 1, status);
  }

  int observation_logger::report_action_taken(const char* event_id, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();

    auto evt_fn = std::bind(
      [](outcome_event& out_evt, api_status* status, outcome_event in_evt)->int {
        out_evt = std::move(in_evt);
        return error_code::success;
      },
      _1,
      _2,
      outcome_event::report_action_taken(event_id, now)
    );
    return append(std::move(evt_fn), event_id, 1, status);
  }

  int generic_event_logger::log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, event_content_type content_type, api_status* status) {
    generic_event::object_list_t objects;
    return log(event_id, std::move(payload), type, content_type, std::move(objects), status);
  }

  int generic_event_logger::log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, event_content_type content_type, generic_event::object_list_t&& objects, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    // TODO: The functor needs to do the heavy lifting. Just put a placeholder here for now
    auto evt_fn = std::bind(
      [](generic_event& out_evt, api_status* status, generic_event in_evt)->int {
        out_evt = std::move(in_evt);
        return error_code::success;
      },
      _1,
      _2,
      std::move(generic_event(event_id, now, type, std::move(payload), content_type, std::move(objects), _app_id))
    );
    return append(std::move(evt_fn), event_id, 1, status);
  }

}}
