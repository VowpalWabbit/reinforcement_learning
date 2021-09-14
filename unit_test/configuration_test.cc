#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "configuration.h"
#include "config_utility.h"
#include "api_status.h"
#include "constants.h"

namespace util = reinforcement_learning::utility;
namespace err = reinforcement_learning::error_code;

const auto config_json = R"({
    "StringKey": "StringValue",
    "IntKey": 7,
    "FloatKey": 3.67,
    "BooleanKey": true
  })";

BOOST_AUTO_TEST_CASE(configuration_set_value) {
  util::configuration config;
  config.set("CustomKey", "CustomValue");
  BOOST_CHECK_EQUAL(config.get("CustomKey", "DefaultValue"), "CustomValue");
}

BOOST_AUTO_TEST_CASE(configuration_get_string_value) {
  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(config.get("StringKey", "DefaultValue"), "StringValue");
}

BOOST_AUTO_TEST_CASE(configuration_get_default_value) {
  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(config.get("NonExistantKey", "DefaultValue"), "DefaultValue");
}

BOOST_AUTO_TEST_CASE(configuration_get_int_value) {
  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(config.get_int("IntKey", 0), 7);
}

BOOST_AUTO_TEST_CASE(configuration_get_float_value) {
  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_CLOSE(config.get_float("FloatKey", 0.0), 3.67, 0.0001);
}

BOOST_AUTO_TEST_CASE(configuration_get_bool_value) {
  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(config.get_bool("BooleanKey", false), true);
}

BOOST_AUTO_TEST_CASE(configuration_parse_get_translated_name) {
  const auto json = R"({
      "ApplicationID": "application_id_value"
  })";
  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(json, config), err::success);
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::APP_ID, "DefaultValue"), "application_id_value");
  BOOST_CHECK_EQUAL(config.get("ApplicationID", "DefaultValue"), "DefaultValue");
}

BOOST_AUTO_TEST_CASE(configuration_parse_get_untranslated_name) {
  const auto json = R"({
      "interaction.eventhub.tasks_limit": 6
  })";
  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(json, config), err::success);
  BOOST_CHECK_EQUAL(config.get_int(reinforcement_learning::name::INTERACTION_EH_TASKS_LIMIT, 0), 6);
}

BOOST_AUTO_TEST_CASE(configuration_parse_array_expect_error) {
  const auto json = R"({ malformed_json })";
  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(json, config), err::json_parse_error);
}

BOOST_AUTO_TEST_CASE(configuration_parse_eventhub_connection_string) {
  const auto json = R"({
    "EventHubInteractionConnectionString": "Endpoint=sb://<ingest>.servicebus.windows.net/;SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=<SAKey>;EntityPath=interaction"
  })";

  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(json, config), err::success);
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::INTERACTION_EH_HOST, "DefaultValue"), "<ingest>.servicebus.windows.net");
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::INTERACTION_EH_NAME, "DefaultValue"), "interaction");
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::INTERACTION_EH_KEY_NAME, "DefaultValue"), "RootManageSharedAccessKey");
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::INTERACTION_EH_KEY, "DefaultValue"), "<SAKey>");
}

BOOST_AUTO_TEST_CASE(configuration_parse_interaction_observation_config) {
  const auto json = R"({
    "http.api.key": "api_key",
    "interaction.http.api.host": "http://localhost:8080",
    "interaction.apim.tasks_limit": 5,
    "interaction.apim.max_http_retries": 3,
    "observation.http.api.host": "http://localhost:8080",
    "observation.apim.tasks_limit": 6,
    "observation.apim.max_http_retries": 4,
    "interaction.sender.implementation": "INTERACTION_HTTP_API_SENDER",
    "observation.sender.implementation": "OBSERVATION_HTTP_API_SENDER"
  })";

  util::configuration config;
  BOOST_CHECK_EQUAL(util::config::create_from_json(json, config), err::success);
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::HTTP_API_KEY, "DefaultValue"), "api_key");
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::INTERACTION_HTTP_API_HOST, "DefaultValue"), "http://localhost:8080");
  BOOST_CHECK_EQUAL(config.get_int(reinforcement_learning::name::INTERACTION_APIM_TASKS_LIMIT, 0), 5);
  BOOST_CHECK_EQUAL(config.get_int(reinforcement_learning::name::INTERACTION_APIM_MAX_HTTP_RETRIES, 0), 3);
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::OBSERVATION_HTTP_API_HOST, "DefaultValue"), "http://localhost:8080");
  BOOST_CHECK_EQUAL(config.get_int(reinforcement_learning::name::OBSERVATION_APIM_TASKS_LIMIT, 0), 6);
  BOOST_CHECK_EQUAL(config.get_int(reinforcement_learning::name::OBSERVATION_APIM_MAX_HTTP_RETRIES, 0), 4);
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::INTERACTION_SENDER_IMPLEMENTATION, "DefaultValue"), "INTERACTION_HTTP_API_SENDER");
  BOOST_CHECK_EQUAL(config.get(reinforcement_learning::name::OBSERVATION_SENDER_IMPLEMENTATION, "DefaultValue"), "OBSERVATION_HTTP_API_SENDER");
}

BOOST_AUTO_TEST_CASE(configuration_parse_malformed_eventhub_connection_string_expect_error) {
  util::configuration config;

  const auto json1 = R"({"EventHubInteractionConnectionString": "<ingest>.servicebus.windows.net/;SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=<SAKey>;EntityPath=interaction"})";
  const auto json2 = R"({"EventHubInteractionConnectionString": "Endpoint=sb://<ingest>.servicebus.windows.net/;SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=<SAKey>"})";
  const auto json3 = R"({"EventHubInteractionConnectionString": "Endpoint=sb://<ingest>.servicebus.windows.net/;AccessKeyName=RootManageSharedAccessKey;SharedAccessKey=<SAKey>;EntityPath=interaction"})";
  const auto json4 = R"({"EventHubInteractionConnectionString": "Endpoint=mp://<ingest>.servicebus.windows.net/,SharedAccessKeyName=RootManageSharedAccessKey,SharedAccessKey=<SAKey>,EntityPath=interaction"})";

  BOOST_CHECK_EQUAL(util::config::create_from_json(json1, config), err::eh_connstr_parse_error);
  BOOST_CHECK_EQUAL(util::config::create_from_json(json2, config), err::eh_connstr_parse_error);
  BOOST_CHECK_EQUAL(util::config::create_from_json(json3, config), err::eh_connstr_parse_error);
  BOOST_CHECK_EQUAL(util::config::create_from_json(json4, config), err::eh_connstr_parse_error);
}

BOOST_AUTO_TEST_CASE(configuration_parse_invalid_json_object_element_expect_error) {
  util::configuration config;

  const auto json = R"({
    "ObjectKey": {
      "Ignore": "This"
    }
  })";

  BOOST_CHECK_EQUAL(util::config::create_from_json(json, config), err::json_parse_error);
}

BOOST_AUTO_TEST_CASE(configuration_parse_invalid_json_array_element_expect_error) {
  util::configuration config;

  const auto json = R"({
    "ArrayKey": [
      "IgnoreThis"
    ]
  })";

  BOOST_CHECK_EQUAL(util::config::create_from_json(json, config), err::json_parse_error);
}
