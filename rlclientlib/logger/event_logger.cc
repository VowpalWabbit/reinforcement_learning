#include "event_logger.h"
#include "ranking_event.h"
#include "err_constants.h"
#include "time_helper.h"

#include <functional>
#include <memory>
namespace reinforcement_learning { namespace logger {
using namespace std::placeholders;
  int interaction_logger::log(const char* event_id, string_view context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    // using shared_ptr because we can't move a unique_ptr in C++11
    // We should replace them in C++14
    auto evt_sp = std::make_shared<ranking_event>();
    // A gross little shuffle because report_outcome returns a copy, but we actually need a pointer
    auto evt_copy = ranking_event::choose_rank(event_id, context, flags, response, now, 1.0f, learning_mode);
    *evt_sp = std::move(evt_copy);

    auto evt_fn =
      [evt_sp](ranking_event& out_evt, api_status* status)->int {
        out_evt = std::move(*evt_sp);
        return error_code::success;
      };
    return append(std::move(evt_fn), event_id, evt_sp.get(), status);
    
  }

  int ccb_logger::log_decisions(std::vector<const char*>& event_ids, string_view context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
    const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    auto evt_sp = std::make_shared<decision_ranking_event>();
    auto evt_copy = decision_ranking_event::request_decision(event_ids, context, flags, action_ids, pdfs, model_version, now);
    *evt_sp = std::move(evt_copy);

    const char* evt_id = evt_sp->get_seed_id().c_str();
    auto evt_fn = [evt_sp](decision_ranking_event& out_evt, api_status* status)->int {
      out_evt = std::move(*evt_sp);
      return error_code::success;
    };
    return append(std::move(evt_fn), evt_id, evt_sp.get(), status);
  }

  int multi_slot_logger::log_decision(const std::string &event_id, string_view context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status) {

    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();

    auto evt_sp = std::make_shared<multi_slot_decision_event>();
    auto evt_copy = multi_slot_decision_event::request_decision(event_id, context, flags, action_ids, pdfs, model_version, now);
    *evt_sp = std::move(evt_copy);
    
    auto evt_fn = 
        [evt_sp](multi_slot_decision_event& out_evt, api_status* status)->int {
        out_evt = std::move(*evt_sp);
        return error_code::success;
      };;
    return append(std::move(evt_fn), event_id.c_str(), evt_sp.get(), status);
  }

  int observation_logger::report_action_taken(const char* event_id, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    auto evt_sp = std::make_shared<outcome_event>();
    auto evt_copy = outcome_event::report_action_taken(event_id, now);
    *evt_sp = std::move(evt_copy);

    auto evt_fn =
      [evt_sp](outcome_event& out_evt, api_status* status)->int {
        out_evt = std::move(*evt_sp);
        return error_code::success;
      };
    return append(std::move(evt_fn), event_id, evt_sp.get(), status);
  }

  int generic_event_logger::log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, event_content_type content_type, api_status* status) {
    generic_event::object_list_t objects;
    return log(event_id, std::move(payload), type, content_type, std::move(objects), status);
  }

  int generic_event_logger::log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, event_content_type content_type, generic_event::object_list_t&& objects, api_status* status) {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    auto evt_sp = std::make_shared<generic_event>(
      event_id, now, type, std::move(payload), content_type, std::move(objects), _app_id
    );
    
    auto evt_fn = [evt_sp](generic_event& out_evt, api_status* status)->int {
      out_evt = std::move(*evt_sp);
      return error_code::success;
    };
    return append(std::move(evt_fn), event_id, evt_sp.get(), status);
  }
}}
