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
