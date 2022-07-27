#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "config_utility.h"
#include "constants.h"
#include "utility/header_authorization.h"

#include <cpprest/http_headers.h>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace err = reinforcement_learning::error_code;
using namespace std::chrono;

BOOST_AUTO_TEST_CASE(apiKey_configuration_test)
{
  const auto config_json = R"({
        "http.api.key": "apikey1234"
  })";
  u::configuration config;
  std::unique_ptr<r::header_authorization> api_obj =
      std::unique_ptr<r::header_authorization>(new r::header_authorization());
  r::api_status status;
  http_headers header;
  BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(api_obj->init(config, &status, nullptr), r::error_code::success);
  BOOST_CHECK_EQUAL(api_obj->insert_authorization_header(header, &status, nullptr), r::error_code::success);
  std::string iter = utility::conversions::to_utf8string(header.find(U("Ocp-Apim-Subscription-Key"))->second);
  BOOST_CHECK_EQUAL(iter, "apikey1234");
}

BOOST_AUTO_TEST_CASE(insert_authorization_header_without_init)
{
  const auto config_json = R"({
        "http.api.key": "apikey1234"
  })";
  u::configuration config;
  std::unique_ptr<r::header_authorization> api_obj =
      std::unique_ptr<r::header_authorization>(new r::header_authorization());
  r::api_status status;
  http_headers header;
  BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(
      api_obj->insert_authorization_header(header, &status, nullptr), r::error_code::http_api_key_not_provided);
}

BOOST_AUTO_TEST_CASE(token_configuration_test)
{
  const auto config_json = R"({
        "http.api.key": "Bearer token1234",
        "http.api.header.key.name" : "Authorization"
  })";
  u::configuration config;
  std::unique_ptr<r::header_authorization> api_obj =
      std::unique_ptr<r::header_authorization>(new r::header_authorization());
  r::api_status status;
  http_headers header;
  BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(api_obj->init(config, &status, nullptr), r::error_code::success);
  BOOST_CHECK_EQUAL(api_obj->insert_authorization_header(header, &status, nullptr), r::error_code::success);
  std::string iter = utility::conversions::to_utf8string(header.find(U("Authorization"))->second);
  BOOST_CHECK_EQUAL(iter, "Bearer token1234");
}

BOOST_AUTO_TEST_CASE(key_type_missing_configuration_test)
{
  const auto config_json = R"({
        "http.api.key": "Beaerer token1234"
  })";
  u::configuration config;
  std::unique_ptr<r::header_authorization> api_obj =
      std::unique_ptr<r::header_authorization>(new r::header_authorization());
  r::api_status status;
  http_headers header;
  BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(api_obj->init(config, &status, nullptr), r::error_code::success);
  BOOST_CHECK_EQUAL(api_obj->insert_authorization_header(header, &status, nullptr), r::error_code::success);
  std::string iter = utility::conversions::to_utf8string(header.find(U("Ocp-Apim-Subscription-Key"))->second);
  BOOST_CHECK_EQUAL(iter, "Beaerer token1234");
}

BOOST_AUTO_TEST_CASE(key_missing_configuration_test)
{
  const auto config_json = R"({
        "http.api.header.key.name" : "Authorization"
  })";
  u::configuration config;
  std::unique_ptr<r::header_authorization> api_obj =
      std::unique_ptr<r::header_authorization>(new r::header_authorization());
  r::api_status status;
  http_headers header;
  BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(api_obj->init(config, &status, nullptr), r::error_code::http_api_key_not_provided);
}

BOOST_AUTO_TEST_CASE(key_type_random_configuration_test)
{
  const auto config_json = R"({
        "http.api.key": "apikey1234",
        "http.api.header.key.name" : "random"
  })";
  u::configuration config;
  std::unique_ptr<r::header_authorization> api_obj =
      std::unique_ptr<r::header_authorization>(new r::header_authorization());
  r::api_status status;
  http_headers header;
  BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
  BOOST_CHECK_EQUAL(api_obj->init(config, &status, nullptr), r::error_code::success);
  BOOST_CHECK_EQUAL(api_obj->insert_authorization_header(header, &status, nullptr), r::error_code::success);
  std::string iter = utility::conversions::to_utf8string(header.find(U("random"))->second);
  BOOST_CHECK_EQUAL(iter, "apikey1234");
}
