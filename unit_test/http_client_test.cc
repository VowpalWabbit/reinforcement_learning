#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "utility/http_client.h"

#include <unordered_map>
#include <vector>

#include "api_status.h"
#include "config_utility.h"
#include "configuration.h"
#include "constants.h"
#include "err_constants.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace e = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

const auto JSON_CFG = R"(
  {
    "ApplicationID": "rnc-123456-a",
    "EventHubInteractionConnectionString": "Endpoint=sb://localhost:8080/;SharedAccessKeyName=RMSAKey;SharedAccessKey=<ASharedAccessKey>=;EntityPath=interaction",
    "EventHubObservationConnectionString": "Endpoint=sb://localhost:8080/;SharedAccessKeyName=RMSAKey;SharedAccessKey=<ASharedAccessKey>=;EntityPath=observation",
    "IsExplorationEnabled": true,
    "ModelBlobUri": "http://localhost:8080",
    "InitialExplorationEpsilon": 1.0
  }
  )";

BOOST_AUTO_TEST_CASE(http_client_init_validation_test) {
  u::configuration cc;
  const auto scode = cfg::create_from_json(JSON_CFG, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);

  std::unique_ptr<r::i_http_client> client;
  const auto result = r::create_http_client("invalid_url", cc, client);
  BOOST_CHECK_EQUAL(result, r::error_code::http_client_init_error);
}

BOOST_AUTO_TEST_CASE(http_client_init_and_encode) {
  u::configuration cc;
  const auto scode = cfg::create_from_json(JSON_CFG, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);

  std::unique_ptr<r::i_http_client> client;
  const std::string url{"http://localhost:8080/_test&key=v@lue?"};
  const auto result = r::create_http_client(url.c_str(), cc, client);
  BOOST_CHECK_EQUAL(result, r::error_code::success);
  BOOST_CHECK_EQUAL(client->get_url(), url);

  const std::string expected_encoded_url{
      "http%3A%2F%2Flocalhost%3A8080%2F_test%26key%3Dv%40lue%3F"};
  const auto encoded_url = client->encode(url);
  BOOST_CHECK_EQUAL(encoded_url, expected_encoded_url);

  const std::string expected_encoded_data{
      "aHR0cDovL2xvY2FsaG9zdDo4MDgwL190ZXN0JmtleT12QGx1ZT8%3D"};
  std::vector<unsigned char> data(url.begin(), url.end());
  const auto encoded_data = client->encode(data);
  BOOST_CHECK_EQUAL(encoded_data, expected_encoded_data);
}
