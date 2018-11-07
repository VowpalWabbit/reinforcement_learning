#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "configuration.h"
#include "config_utility.h"

namespace util = reinforcement_learning::utility;

BOOST_AUTO_TEST_CASE(configuration_set_value) {
  util::configuration config;

  //self.config.set("CustomKey", "CustomValue")
  //  self.assertEqual(self.config.get("CustomKey", None), "CustomValue")

}

BOOST_AUTO_TEST_CASE(configuration_get_string_value) {
}

BOOST_AUTO_TEST_CASE(configuration_get_default_value) {
}

BOOST_AUTO_TEST_CASE(configuration_get_int_value) {
}

BOOST_AUTO_TEST_CASE(configuration_get_float_value) {
}

BOOST_AUTO_TEST_CASE(configuration_get_bool_value) {
}

BOOST_AUTO_TEST_CASE(configuration_parse_get_translated_name) {
  const auto config_json = R"({
      "ApplicationID": "rnc-123456-a",
      "EventHubInteractionConnectionString": "Endpoint=sb://localhost:8080/;SharedAccessKeyName=RMSAKey;SharedAccessKey=<ASharedAccessKey>=;EntityPath=interaction",
      "EventHubObservationConnectionString": "Endpoint=sb://localhost:8080/;SharedAccessKeyName=RMSAKey;SharedAccessKey=<ASharedAccessKey>=;EntityPath=observation",
      "IsExplorationEnabled": true,
      "ModelBlobUri": "http://localhost:8080",
      "InitialExplorationEpsilon": 1.0
  })";
  util::configuration config;
  auto config_obj = util::config::create_from_json(config_json, config);

  //BOOST_CHECK_EQUAL("blah", err::success);
}

BOOST_AUTO_TEST_CASE(configuration_parse_get_untranslated_name) {
}

BOOST_AUTO_TEST_CASE(configuration_parse_array_expect_error) {
}

BOOST_AUTO_TEST_CASE(configuration_parse_eventhub_connection_string) {
}

BOOST_AUTO_TEST_CASE(configuration_parse_malformed_eventhub_connection_string_expect_error) {
}
