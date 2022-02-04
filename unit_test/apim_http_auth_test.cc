#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "utility/apim_http_authorization.h"
#include "constants.h"
#include "config_utility.h"
#include <cpprest/http_headers.h>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace err = reinforcement_learning::error_code;
using namespace std::chrono;

BOOST_AUTO_TEST_CASE(apiKey_configuration_test) {
    const auto config_json = R"({
        "http.api.key": "apikey1234",
       "http.key.type": "apiKey"
  })";
    u::configuration config;
    r::apim_http_authorization* apiObj = new r::apim_http_authorization();
    r::api_status status;
    http_headers header;
    BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
    BOOST_CHECK_EQUAL(apiObj->init(config, &status, nullptr), r::error_code::success);
    BOOST_CHECK_EQUAL(apiObj->get_http_headers(header, &status), r::error_code::success);
    std::string iter = utility::conversions::to_utf8string(header.find(U("Ocp-Apim-Subscription-Key"))->second);
    BOOST_CHECK_EQUAL(iter, "apikey1234");
}

BOOST_AUTO_TEST_CASE(token_configuration_test) {
    const auto config_json = R"({
        "http.api.key": "token1234",
       "http.key.type": "Bearer"
  })";
    u::configuration config;
    r::apim_http_authorization* apiObj = new r::apim_http_authorization();
    r::api_status status;
    http_headers header;
    BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
    BOOST_CHECK_EQUAL(apiObj->init(config, &status, nullptr), r::error_code::success);
    BOOST_CHECK_EQUAL(apiObj->get_http_headers(header, &status), r::error_code::success);
    std::string iter = utility::conversions::to_utf8string(header.find(U("Authorization"))->second);
    BOOST_CHECK_EQUAL(iter, "Bearer token1234");
}

BOOST_AUTO_TEST_CASE(key_type_missing_configuration_test) {
    const auto config_json = R"({
        "http.api.key": "token1234"
  })";
    u::configuration config;
    r::apim_http_authorization* apiObj = new r::apim_http_authorization();
    r::api_status status;
    http_headers header;
    BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
    BOOST_CHECK_EQUAL(apiObj->init(config, &status, nullptr), r::error_code::http_auth_type_not_provided);
}

BOOST_AUTO_TEST_CASE(key_missing_configuration_test) {
    const auto config_json = R"({
       "http.key.type": "APIKEY"
  })";
    u::configuration config;
    r::apim_http_authorization* apiObj = new r::apim_http_authorization();
    r::api_status status;
    http_headers header;
    BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
    BOOST_CHECK_EQUAL(apiObj->init(config, &status, nullptr), r::error_code::http_api_key_not_provided);
}

BOOST_AUTO_TEST_CASE(key_type_random_configuration_test) {
    const auto config_json = R"({
        "http.api.key": "apikey1234",
       "http.key.type": "default"
  })";
    u::configuration config;
    r::apim_http_authorization* apiObj = new r::apim_http_authorization();
    r::api_status status;
    http_headers header;
    BOOST_CHECK_EQUAL(u::config::create_from_json(config_json, config), err::success);
    BOOST_CHECK_EQUAL(apiObj->init(config, &status, nullptr), r::error_code::http_auth_type_not_provided);
}

