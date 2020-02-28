#include <map>
#include <set>
#include <regex>
#include "config_utility.h"
#include "cpprest/json.h"
#include "configuration.h"
#include "constants.h"
#include "err_constants.h"
#include "api_status.h"
#include "str_util.h"
#include "trace_logger.h"
#include "../model_mgmt/restapi_data_transport.h"

using namespace web;
using namespace utility::conversions; // string conversions utilities

namespace reinforcement_learning { namespace utility { namespace config {
  std::string load_config_json() {
    //TODO: Load appid configuration from Azure storage
    //TODO: error handling.  (return code or exception)
    return "";
  }

  const char* regex_code_str(int code) {
    switch(code) {
    case std::regex_constants::error_collate:
      return "The expression contained an invalid collating element name.";
    case std::regex_constants::error_ctype:
      return "The expression contained an invalid character class name.";
    case std::regex_constants::error_escape:
      return "The expression contained an invalid escaped character, or a trailing escape.";
    case std::regex_constants::error_backref:
      return "The expression contained an invalid back reference.";
    case std::regex_constants::error_brack:
      return "The expression contained mismatched brackets([and]).";
    case std::regex_constants::error_paren:
      return "The expression contained mismatched parentheses(( and )).";
    case std::regex_constants::error_brace:
      return "The expression contained mismatched braces({ and }).";
    case std::regex_constants::error_badbrace:
      return "The expression contained an invalid range between braces({ and }).";
    case std::regex_constants::error_range:
      return "The expression contained an invalid character range.";
    case std::regex_constants::error_space:
      return "There was insufficient memory to convert the expression into a finite state machine.";
    case std::regex_constants::error_badrepeat:
      return "The expression contained a repeat specifier(one of * ? +{) that was not preceded by a valid regular expression.";
    case std::regex_constants::error_complexity:
      return "The complexity of an attempted match against a regular expression exceeded a pre - set level.";
    case std::regex_constants::error_stack:
      return "There was insufficient memory to determine whether the regular expression could match the specified character sequence.";
    default:
      return "Undefined regex error";
    }
  }

  const char* error_code_str(int err_code)
  {
    switch(err_code){
      case error_code::success:
        return "Success";
      case error_code::invalid_argument:
        RETURN_ERROR_MSG(invalid_argument);
      case error_code::background_queue_overflow:
        RETURN_ERROR_MSG(background_queue_overflow);
      case error_code::eventhub_http_generic:
        RETURN_ERROR_MSG(eventhub_http_generic);
      case error_code::http_bad_status_code:
        RETURN_ERROR_MSG(http_bad_status_code);
      case error_code::action_not_found:
        RETURN_ERROR_MSG(action_not_found);
      case error_code::background_thread_start:
        RETURN_ERROR_MSG(background_thread_start);
      case error_code::not_initialized:
        RETURN_ERROR_MSG(not_initialized);
      case error_code::eventhub_generate_SAS_hash:
        RETURN_ERROR_MSG(eventhub_generate_SAS_hash);
      case error_code::create_fn_exception:
        RETURN_ERROR_MSG(create_fn_exception);
      case error_code::type_not_registered:
        RETURN_ERROR_MSG(type_not_registered);
      case error_code::http_uri_not_provided:
        RETURN_ERROR_MSG(http_uri_not_provided);
      case error_code::last_modified_not_found:
        RETURN_ERROR_MSG(last_modified_not_found);
      case error_code::last_modified_invalid:
        RETURN_ERROR_MSG(last_modified_invalid);
      case error_code::bad_content_length:
        RETURN_ERROR_MSG(bad_content_length);
      case error_code::exception_during_http_req:
        RETURN_ERROR_MSG(exception_during_http_req);
      case error_code::model_export_frequency_not_provided:
        RETURN_ERROR_MSG(model_export_frequency_not_provided);
      case error_code::bad_time_interval:
        RETURN_ERROR_MSG(bad_time_interval);
      case error_code::data_callback_exception:
        RETURN_ERROR_MSG(data_callback_exception);
      case error_code::data_callback_not_set:
        RETURN_ERROR_MSG(data_callback_not_set);
      case error_code::json_no_actions_found:
        RETURN_ERROR_MSG(json_no_actions_found);
      case error_code::json_parse_error:
        RETURN_ERROR_MSG(json_parse_error);
      case error_code::exploration_error:
        RETURN_ERROR_MSG(exploration_error);
      case error_code::action_out_of_bounds:
        RETURN_ERROR_MSG(action_out_of_bounds);
      case error_code::model_update_error:
        RETURN_ERROR_MSG(model_update_error);
      case error_code::model_rank_error:
        RETURN_ERROR_MSG(model_rank_error);
      case error_code::pdf_sampling_error:
        RETURN_ERROR_MSG(pdf_sampling_error);
      case error_code::eh_connstr_parse_error:
        RETURN_ERROR_MSG(eh_connstr_parse_error);
      case error_code::unhandled_background_error_occurred:
        RETURN_ERROR_MSG(unhandled_background_error_occurred);
      case error_code::thread_unresponsive_timeout:
        RETURN_ERROR_MSG(thread_unresponsive_timeout);
      case error_code::incorrect_buffer_preamble_size:
        RETURN_ERROR_MSG(incorrect_buffer_preamble_size);
      case error_code::serialize_unknown_outcome_type:
        RETURN_ERROR_MSG(serialize_unknown_outcome_type);
      case error_code::preamble_error:
        RETURN_ERROR_MSG(preamble_error);
      case error_code::file_open_error:
        RETURN_ERROR_MSG(file_open_error);
      case error_code::json_no_slots_found:
        RETURN_ERROR_MSG(json_no_slots_found);
      case error_code::file_read_error:
        RETURN_ERROR_MSG(file_read_error);
      case error_code::file_stats_error:
        RETURN_ERROR_MSG(file_stats_error);
      case error_code::not_supported:
        RETURN_ERROR_MSG(not_supported);
      default:
        return "Unexpected Error";
    }
  }
  
  int parse_eventhub_conn_str(const std::string& conn_str, std::string& host, std::string& name, std::string& access_key_name, std::string& access_key, i_trace* trace, api_status* status) {
    try {
      const std::regex regex_eh_connstr("Endpoint=sb://([^/]+)[^;]+;SharedAccessKeyName=([^;]+);SharedAccessKey=([^;]+);EntityPath=([^;^\\s]+)");
      std::smatch match;
      if ( !std::regex_match(conn_str, match, regex_eh_connstr) && !( match.size() == 5 ) ) {
        RETURN_ERROR_LS(trace, status, eh_connstr_parse_error) << conn_str;
      }
      host = match[1].str();
      access_key_name = match[2].str();
      access_key = match[3].str();
      name = match[4].str();
      return error_code::success;

    }
    catch ( const std::regex_error& e) {
      RETURN_ERROR_LS(trace, status, eh_connstr_parse_error) << conn_str << ", regex_error: " << regex_code_str(e.code()) << ", details: " << e.what();
    }
  }

  int set_eventhub_config(const std::string& conn_str, const std::string& cfg_root, configuration& cc, i_trace* trace,  api_status* status) {
    std::string host;
    std::string name;
    std::string access_key_name;
    std::string access_key;
    RETURN_IF_FAIL(parse_eventhub_conn_str(conn_str, host, name, access_key_name, access_key, trace, status));
    const auto topic = ".eventhub.";
    cc.set(concat(cfg_root, topic, "host").c_str(),     host.c_str());
    cc.set(concat(cfg_root, topic, "name").c_str(),     name.c_str());
    cc.set(concat(cfg_root, topic, "keyname").c_str(), access_key_name.c_str());
    cc.set(concat(cfg_root, topic, "key").c_str(),      access_key.c_str());
    return error_code::success;
  }

  std::string translate(const std::map<std::string, std::string>& from_to, const std::string& from) {
    const auto it = from_to.find(from);
    if ( it != from_to.end() )
      return it->second;
    return from;
  }

  int create_from_json(const std::string& config_json, configuration& cc, i_trace* trace, api_status* status) {
    static const std::map<std::string, std::string> legacy_translation_mapping = {
      { "ApplicationID"             , name::APP_ID },
      { "ModelBlobUri"              , name::MODEL_BLOB_URI },
      { "SendHighMaterMark"         , name::INTERACTION_SEND_HIGH_WATER_MARK },
      { "SendBatchIntervalMs"       , name::INTERACTION_SEND_BATCH_INTERVAL_MS },
      { "InitialExplorationEpsilon" , name::INITIAL_EPSILON },
      { "ModelRefreshIntervalMs"    , name::MODEL_REFRESH_INTERVAL_MS },
      { "QueueMode"                 , name::QUEUE_MODE } // expect either DROP or BLOCK, default is DROP
    };

    const std::set<std::string> deprecated = {
     "QueueMaxSize"
    };

    web::json::value obj;
    try {
      obj = json::value::parse(to_string_t(config_json));
    }
    catch (const web::json::json_exception& e) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << e.what();
    }
    catch (const std::exception& e) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << e.what();
    }

    auto jsonObj = obj.as_object();

    for (auto const& prop_pair : jsonObj) {
      auto prop_name = to_utf8string(prop_pair.first);
      auto const& prop_value = prop_pair.second;
      auto const string_value = to_utf8string(prop_value.is_string() ? prop_value.as_string() : prop_value.serialize());
      prop_name = translate(legacy_translation_mapping, prop_name);
      if (deprecated.find(prop_name) != deprecated.end()) {
        auto message = concat("Field '", prop_name, "' is unresponsive.");
        TRACE_WARN(trace, message);
      }
      // Check if the current field is an EventHub connect string that needs to be parsed.
      if (prop_name == "EventHubInteractionConnectionString") {
        RETURN_IF_FAIL(set_eventhub_config(string_value, "interaction", cc, trace, status));
      }
      else if (prop_name == "EventHubObservationConnectionString") {
        RETURN_IF_FAIL(set_eventhub_config(string_value, "observation", cc, trace, status));
      }
      else {
        // Otherwise, just set the value in the config collection.
        cc.set(prop_name.c_str(), string_value.c_str());
      }
    }

    return error_code::success;
  }

}}}
