#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "utility/cpprest_http_client.h"

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

BOOST_AUTO_TEST_CASE(cpprest_http_responses) {
  const std::string resp_data{"HTTP response"};
  std::string resp_str;
  auto resp_buffer = new char[resp_data.length()];
  size_t resp_buffer_sz;

  r::cpprest_http_response resp1(std::move(
      web::http::http_response(web::http::status_codes::InternalError)));
  BOOST_CHECK_EQUAL(resp1.status_code(),
                    r::http_response::status::INTERNAL_ERROR);
  BOOST_CHECK_EQUAL(resp1.content_length(), 0);
  BOOST_CHECK_EQUAL(resp1.last_modified(resp_str), e::last_modified_not_found);
  BOOST_CHECK_EQUAL(resp1.body(resp_buffer_sz, resp_buffer),
                    e::http_response_read_error);

  web::http::http_response tmp(web::http::status_codes::OK);
  tmp.set_body(resp_data);
  r::cpprest_http_response resp2(std::move(tmp));
  BOOST_CHECK_EQUAL(resp2.status_code(), r::http_response::status::OK);
  BOOST_CHECK_EQUAL(resp2.content_length(), resp_data.length());
  BOOST_CHECK_EQUAL(resp2.body(resp_buffer_sz, resp_buffer), e::success);
  BOOST_CHECK_EQUAL(resp_buffer_sz, resp_data.length());
  BOOST_CHECK_EQUAL(std::string(resp_buffer), resp_data);

  delete[] resp_buffer;
}

BOOST_AUTO_TEST_CASE(cpprest_http_client_encode) {
  u::configuration cc;
  const auto scode = cfg::create_from_json(JSON_CFG, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);

  const std::string url{"http://localhost:8080/_test&key=v@lue?"};
  r::cpprest_http_client client(url.c_str(), cc);
  BOOST_CHECK_EQUAL(client.get_url(), url);

  const std::string expected_encoded_url{
      "http%3A%2F%2Flocalhost%3A8080%2F_test%26key%3Dv%40lue%3F"};
  const auto encoded_url = client.encode(url);
  BOOST_CHECK_EQUAL(encoded_url, expected_encoded_url);

  const std::string expected_encoded_data{
      "aHR0cDovL2xvY2FsaG9zdDo4MDgwL190ZXN0JmtleT12QGx1ZT8%3D"};
  std::vector<unsigned char> data(url.begin(), url.end());
  const auto encoded_data = client.encode(data);
  BOOST_CHECK_EQUAL(encoded_data, expected_encoded_data);
}
