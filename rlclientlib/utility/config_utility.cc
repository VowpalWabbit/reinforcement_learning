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
      { "QueueMode"                 , name::QUEUE_MODE }, // expect either DROP or BLOCK, default is DROP
      { "DecisionMode"              , name::DECISION_RANK_MODE}
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
