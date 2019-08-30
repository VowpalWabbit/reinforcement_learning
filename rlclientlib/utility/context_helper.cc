#include <cpprest/json.h>
#include <object_factory.h>
#include <cpprest/asyncrt_utils.h>
#include "err_constants.h"
#include "http_helper.h"

#include <chrono>
#include <cstring>

namespace sutil = ::utility::conversions;

namespace reinforcement_learning { namespace utility {
  const auto multi = sutil::to_string_t("_multi");
  const auto slots = sutil::to_string_t("_slots");
  const auto event_id = sutil::to_string_t("_id");
  /**
   * \brief Get the number of actions found in context json string.  Actions should be in an array
   * called _multi under the root name space.  {_multi:[{"name1":"val1"},{"name1":"val1"}]}
   *
   * \param count   : Return value passed in as a reference.
   * \param context : String with context json
   * \param status  : Pointer to api_status object that contains an error code and error description in
   *                  case of failure
   * \return  error_code::success if there are no errors.  If there are errors then the error code is
   *          returned.
   */
  int get_action_count(size_t& count, const char *context, i_trace* trace, api_status* status) {
    try {
      const auto scontext = sutil::to_string_t(std::string(context));
      auto json_obj = web::json::value::parse(scontext);
      if ( json_obj.has_array_field(multi) ) {
        auto const arr = json_obj.at(multi).as_array();
        count = arr.size();
        if ( count > 0 )
          return reinforcement_learning::error_code::success;
        RETURN_ERROR_LS(trace, status, json_no_actions_found);
      }
      RETURN_ERROR_LS(trace, status, json_no_actions_found);
    }
    catch ( const std::exception& e ) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << error_code::unknown_s;
    }
  }

  int get_event_ids(std::map<size_t, std::string>& event_ids, const char *context, i_trace* trace, api_status* status) {
    try {
      const auto scontext = sutil::to_string_t(std::string(context));
      auto json_obj = web::json::value::parse(scontext);
      if ( json_obj.has_array_field(slots) ) {
        auto const arr = json_obj.at(slots).as_array();
        for (int i = 0; i < arr.size(); i++) {
          auto current = arr.at(i);
          if(current.has_string_field(event_id))
          {
            auto event_id_string = current.at(event_id).as_string();
            event_ids[i] = std::string{ event_id_string.begin(), event_id_string.end() };
          }
        }
        return reinforcement_learning::error_code::success;
      }
      RETURN_ERROR_LS(trace, status, json_no_slots_found);
    }
    catch ( const std::exception& e ) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << error_code::unknown_s;
    }
  }

  int get_slot_count(size_t& count, const char *context, i_trace* trace, api_status* status) {
    try {
      const auto scontext = sutil::to_string_t(std::string(context));
      auto json_obj = web::json::value::parse(scontext);
      if (json_obj.has_array_field(slots)) {
        auto const arr = json_obj.at(slots).as_array();
        count = arr.size();
        if ( count > 0 )
          return reinforcement_learning::error_code::success;
        RETURN_ERROR_LS(trace, status, json_no_slots_found);
      }
      RETURN_ERROR_LS(trace, status, json_no_slots_found);
    }
    catch ( const std::exception& e ) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << error_code::unknown_s;
    }
  }

  int validate_multi_before_slots(const char *context, i_trace* trace, api_status* status)
  {
    auto slots_pos = strstr(context, "_slots");
    auto multi_pos = strstr(context, "_multi");

    if(slots_pos != nullptr && multi_pos != nullptr && slots_pos < multi_pos)
    {
      RETURN_ERROR_LS(trace, status, json_parse_error) << " There must be both a _multi field and _slots, and _multi must come first.";
    }

    return reinforcement_learning::error_code::success;
  }
}}
