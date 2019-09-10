#include <unordered_map>
#include <unordered_set>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>
#include <regex>
#include "config_utility.h"
#include "configuration.h"
#include "constants.h"
#include "err_constants.h"
#include "api_status.h"
#include "str_util.h"
#include "trace_logger.h"

namespace reinforcement_learning { namespace utility { namespace config {
  namespace rj = rapidjson;

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

  int translate_property(std::string prop_name, const char* string_value, configuration& cc, i_trace* trace, api_status* status) {
    // TODO: Remove explicit lower-case translation mappings. This is not completely trivial, because we need to
    // support UTF-8-encoded strings, and naive to_lower() will cause issues for code-points outside of ASCII7.
    static const std::unordered_map<std::string, std::string> legacy_translation_mapping = {
      { "ApplicationID"             , name::APP_ID },
      { "applicationID"             , name::APP_ID },
      { "ModelBlobUri"              , name::MODEL_BLOB_URI },
      { "modelBlobUri"              , name::MODEL_BLOB_URI },
      { "SendHighMaterMark"         , name::INTERACTION_SEND_HIGH_WATER_MARK },
      { "sendHighMaterMark"         , name::INTERACTION_SEND_HIGH_WATER_MARK },
      { "SendBatchIntervalMs"       , name::INTERACTION_SEND_BATCH_INTERVAL_MS },
      { "sendBatchIntervalMs"       , name::INTERACTION_SEND_BATCH_INTERVAL_MS },
      { "InitialExplorationEpsilon" , name::INITIAL_EPSILON },
      { "initialExplorationEpsilon" , name::INITIAL_EPSILON },
      { "ModelRefreshIntervalMs"    , name::MODEL_REFRESH_INTERVAL_MS },
      { "modelRefreshIntervalMs"    , name::MODEL_REFRESH_INTERVAL_MS },
      { "QueueMode"                 , name::QUEUE_MODE }, // expect either DROP or BLOCK, default is DROP
      { "queueMode"                 , name::QUEUE_MODE }, // expect either DROP or BLOCK, default is DROP
      { "LearningMode"              , name::LEARNING_MODE},
      { "learningMode"              , name::LEARNING_MODE},
      { "InitialCommandLine"        , name::MODEL_VW_INITIAL_COMMAND_LINE},
      { "initialCommandLine"        , name::MODEL_VW_INITIAL_COMMAND_LINE},
      { "ProtocolVersion"           , name::PROTOCOL_VERSION},
      { "protocolVersion"           , name::PROTOCOL_VERSION}
    };

    static const std::unordered_map<std::string, std::string> parsed_translation_mapping = {
      { "EventHubInteractionConnectionString" , "interaction" },
      { "eventHubInteractionConnectionString" , "interaction" },
      { "EventHubObservationConnectionString" , "observation" },
      { "eventHubObservationConnectionString" , "observation" }
    };

    static const std::unordered_set<std::string> deprecated = {
     "QueueMaxSize", 
     "queueMaxSize"
    };

    const auto legacy_it = legacy_translation_mapping.find(prop_name);
    if (legacy_it != legacy_translation_mapping.end())
    {
      prop_name = legacy_it->second;
    }

    if (trace != nullptr && deprecated.find(prop_name) != deprecated.end()) 
    {
      auto message = concat("Field '", prop_name, "' is unresponsive.");
      TRACE_WARN(trace, message);
    }

    // Check if the current field is an EventHub connect string that needs to be parsed.
    const auto parsed_it = parsed_translation_mapping.find(prop_name);
    if (parsed_it != parsed_translation_mapping.end())
    {
      RETURN_IF_FAIL(set_eventhub_config(string_value, parsed_it->second, cc, trace, status));
    }
    else
    {
        // Otherwise, just set the value in the config collection.
        cc.set(prop_name.c_str(), string_value);
    }
    
    return error_code::success;
  }

  // TODO: There was an attempt previously to make this work using case-insensitive hashing in std:map (and possibly switch to unordered_map)
  int create_from_json(const std::string& config_json, configuration& cc, i_trace* trace, api_status* status) {
    rj::Document obj;
    try {
      obj.Parse<rj::kParseNumbersAsStringsFlag | rj::kParseDefaultFlags>(config_json.c_str());
      if (obj.HasParseError()) {
        RETURN_ERROR_LS(trace, status, json_parse_error) << "JSON parse error: " << rj::GetParseError_En(obj.GetParseError()) << " (" << obj.GetErrorOffset() << ")";
      }
    }
    catch (const std::exception& e) {
      RETURN_ERROR_LS(trace, status, json_parse_error) << e.what();
    }

    auto const& jsonObj = obj.GetObject();

    for (auto const& prop_pair : jsonObj) {
      auto prop_name_raw = prop_pair.name.GetString();
      auto prop_name = std::string(prop_name_raw);
      auto const& prop_value = prop_pair.value;

      const char *string_value;
      if (prop_value.IsString()) {
        string_value = prop_value.GetString();
      }
      else if (prop_value.IsBool()) {
        string_value = prop_value.GetBool() ? "true" : "false";
      }
      else {
        RETURN_ERROR_LS(trace, status, json_parse_error) << "Invalid json type found: " << prop_value.GetType();
      }

      RETURN_IF_FAIL(translate_property(prop_name, string_value, cc, trace, status));
    }

    return error_code::success;
  }

}}}
