#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <unordered_map>
#include "model_mgmt.h"
#include "object_factory.h"
#include "factory_resolver.h"
#include "constants.h"
#include "api_status.h"
#include "err_constants.h"
#include <regex>
#include "utility/periodic_background_proc.h"
#include "model_mgmt/model_downloader.h"
#include "model_mgmt/data_callback_fn.h"
#include "config_utility.h"
#include "configuration.h"
#include "utility/watchdog.h"

#ifdef USE_AZURE_FACTORIES
#   include "model_mgmt/restapi_data_transport.h"
#   include "mock_http_client.h"
#endif

namespace r = reinforcement_learning;
namespace m = reinforcement_learning::model_management;
namespace u =  reinforcement_learning::utility;
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
const auto JSON_CONTEXT = R"({"_multi":[{},{}]})";

