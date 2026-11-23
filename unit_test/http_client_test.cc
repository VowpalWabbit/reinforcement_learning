#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "api_status.h"
#include "config_utility.h"
#include "configuration.h"
#include "constants.h"
#include "err_constants.h"
#include "utility/http_client.h"
#include "utility/http_helper.h"

#include <limits>
#include <unordered_map>

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

BOOST_AUTO_TEST_CASE(http_client_init_validation_test)
{
  u::configuration cc;
  const auto scode = cfg::create_from_json(JSON_CFG, cc);
  BOOST_CHECK_EQUAL(scode, r::error_code::success);

  r::i_http_client* client;
  const auto result = r::create_http_client("invalid_url", cc, &client);
  BOOST_CHECK_EQUAL(result, r::error_code::http_client_init_error);
}

BOOST_AUTO_TEST_CASE(http_client_timeout_default)
{
  u::configuration cc;
  BOOST_CHECK_EQUAL(u::get_http_config(cc).timeout().count(), 30);
}

BOOST_AUTO_TEST_CASE(http_client_timeout_positive_values)
{
  for (const int timeout : {1, 30, 31, 120, 3600, (std::numeric_limits<int>::max)()})
  {
    BOOST_TEST_CONTEXT("http.timeout = " << timeout)
    {
      u::configuration cc;
      cc.set(r::name::HTTP_CLIENT_TIMEOUT, std::to_string(timeout).c_str());
      BOOST_CHECK_EQUAL(u::get_http_config(cc).timeout().count(), timeout);
    }
  }
}

BOOST_AUTO_TEST_CASE(http_client_timeout_invalid_values)
{
  for (const char* timeout : {"0", "-1", "invalid"})
  {
    BOOST_TEST_CONTEXT("http.timeout = " << timeout)
    {
      u::configuration cc;
      cc.set(r::name::HTTP_CLIENT_TIMEOUT, timeout);
      BOOST_CHECK_EQUAL(u::get_http_config(cc).timeout().count(), 30);
    }
  }
}
