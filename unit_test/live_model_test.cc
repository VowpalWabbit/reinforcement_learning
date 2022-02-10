#include "multi_slot_response.h"
#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <thread>
#include <boost/test/unit_test.hpp>
#include <vector>

#include "live_model.h"
#include "config_utility.h"
#include "api_status.h"
#include "ranking_response.h"
#include "err_constants.h"
#include "constants.h"
#include "sender.h"
#include "model_mgmt.h"
#include "str_util.h"
#include "factory_resolver.h"
#include "sampling.h"

#include "mock_util.h"

#ifdef USE_AZURE_FACTORIES
#   include "model_mgmt/restapi_data_transport.h"
#   include "mock_http_client.h"
#endif

constexpr float FLOAT_TOL = 0.0001f;
#ifdef __GNUG__

// Fakeit does not work with GCC's devirtualization
// which is enabled with -O2 (the default) or higher.
#pragma GCC optimize("no-devirtualize")

#endif

#include <fakeit/fakeit.hpp>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace m = reinforcement_learning::model_management;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

using namespace fakeit;

namespace {

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

  const auto JSON_CFG_API = R"(
    {
    "ApplicationID": "rnc-123456-b",
    "interaction.http.api.host": "http://localhost:8080/personalizer/v1.1-preview.2/logs/interactions",
    "observation.http.api.host": "http://localhost:8080/personalizer/v1.1-preview.2/logs/observations",
    "IsExplorationEnabled": true,
    "InitialExplorationEpsilon": 1.0,
    "LearningMode": "Online",
    "model.source": "HTTP_MODEL_DATA",
    "observation.sender.implementation":"OBSERVATION_HTTP_API_SENDER",
    "interaction.sender.implementation":"INTERACTION_HTTP_API_SENDER",
    "protocol.version":"2"
  }
  )";

  const auto JSON_CONTEXT = R"({"_multi":[{},{}]})";
  const auto JSON_CONTEXT_WITH_SLOTS = R"({"_multi":[{},{}],"_slots":[{}]})";
  const auto JSON_CONTEXT_WITH_SLOTS_WITH_SLOT_IDS = R"({"_multi":[{},{}],"_slots":[{"_id":"provided_slot_id_1"}, {}]})";
  const auto JSON_CONTEXT_WITH_SLOTS_W_SLOT_IDS_AND_SLOT_NS = R"({"_multi":[{},{}, {}],"_slots":[{"_id":"provided_slot_id_1", "slot_namespace":{"a":"b"}}, {}, {"ns":{"_id":"ignored_slot_id", "a":"b"}}]})";
  const auto JSON_CONTEXT_PDF = R"({"Shared":{"t":"abc"}, "_multi":[{"Action":{"c":1}},{"Action":{"c":2}}],"p":[0.4, 0.6]})";
  const auto JSON_CONTEXT_LEARNING = R"({"Shared":{"t":"abc"}, "_multi":[{"Action":{"c":1}},{"Action":{"c":2}},{"Action":{"c":3}}],"p":[0.4, 0.1, 0.5]})";
  const auto JSON_CONTEXT_CONTINUOUS_ACTIONS = R"({"Temperature":{"18-25":1,"4":1,"C":1,"0":1,"1":1,"2":1,"15":1,"M":1}})";
  const float EXPECTED_PDF[2] = { 0.4f, 0.6f };

  r::live_model create_mock_live_model(
    const u::configuration& config,
    r::data_transport_factory_t* data_transport_factory = nullptr,
    r::model_factory_t* model_factory = nullptr,
    r::sender_factory_t* sender_factory = nullptr,
    r::model_management::model_type_t model_type = r::model_management::model_type_t::UNKNOWN) {

      static auto mock_sender = get_mock_sender(r::error_code::success);
      static auto mock_data_transport = get_mock_data_transport();
      static auto mock_model = get_mock_model(model_type);

      static auto default_sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get());
      static auto default_data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
      static auto default_model_factory = get_mock_model_factory(mock_model.get());

      if (!data_transport_factory) {
          data_transport_factory = default_data_transport_factory.get();
      }

      if (!model_factory) {
          model_factory = default_model_factory.get();
      }

      if (!sender_factory) {
          sender_factory = default_sender_factory.get();
      }

      r::live_model model(config, nullptr, nullptr, &r::trace_logger_factory, data_transport_factory, model_factory, sender_factory);
      return model;
  }

}

BOOST_AUTO_TEST_CASE(schema_v1_with_bad_use_dedup) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::INTERACTION_USE_DEDUP, "true");
  r::api_status status;
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::content_encoding_error);
}

BOOST_AUTO_TEST_CASE(schema_v1_with_use_compression) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::INTERACTION_USE_COMPRESSION, "true");
  r::api_status status;
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::content_encoding_error);
}

BOOST_AUTO_TEST_CASE(schema_v1_with_bad_compression_observations) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::OBSERVATION_USE_COMPRESSION, "true");
  r::api_status status;
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::content_encoding_error);
}

BOOST_AUTO_TEST_CASE(schema_v1_with_global_use_compression) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::USE_COMPRESSION, "true");
  r::api_status status;
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::content_encoding_error);
}

BOOST_AUTO_TEST_CASE(schema_v1_with_global_use_dedup) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::USE_DEDUP, "true");
  r::api_status status;
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::content_encoding_error);
}

BOOST_AUTO_TEST_CASE(schema_v2_with_zstd_and_dedup_content_encoding) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::INTERACTION_USE_COMPRESSION, "true");
  config.set(r::name::INTERACTION_USE_DEDUP, "true");
  r::api_status status;
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);
}

BOOST_AUTO_TEST_CASE(live_model_ranking_request) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  r::api_status status;

  //create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto event_id = "event_id";
  const auto invalid_event_id = "";
  const auto invalid_context = "";

  r::ranking_response response;

  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, JSON_CONTEXT, response), err::success);

  // check expected returned codes
  BOOST_CHECK_EQUAL(ds.choose_rank(invalid_event_id, JSON_CONTEXT, response), err::invalid_argument); // invalid event_id
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, invalid_context, response), err::invalid_argument); // invalid context

  // invalid event_id
  ds.choose_rank(event_id, invalid_context, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);

  //invalid context
  ds.choose_rank(invalid_event_id, JSON_CONTEXT, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);

  //valid request => status is reset
  r::api_status::try_update(&status, -42, "hello");
  ds.choose_rank(event_id, JSON_CONTEXT, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
}

BOOST_AUTO_TEST_CASE(live_model_ranking_request_online_mode) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_ONLINE);

  r::api_status status;

  //create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto event_id = "event_id";
  r::ranking_response response;
  ds.choose_rank(event_id, JSON_CONTEXT_LEARNING, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
}

BOOST_AUTO_TEST_CASE(live_model_ranking_request_apprentice_mode) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_APPRENTICE);

  r::api_status status;

  //create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto event_id = "event_id";

  r::ranking_response response;

  ds.choose_rank(event_id, JSON_CONTEXT_LEARNING, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
  size_t chosen_action;
  response.get_chosen_action_id(chosen_action, &status);
  BOOST_CHECK_EQUAL(chosen_action, 0);
  int current_expect_action_id = 0;
  for (auto it = response.begin(); it != response.end(); ++it) {
    BOOST_CHECK_EQUAL((*it).action_id, current_expect_action_id);
    current_expect_action_id++;
  }
}


BOOST_AUTO_TEST_CASE(live_model_ranking_request_loggingonly_mode) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_LOGGINGONLY);

  r::api_status status;

  //create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto event_id = "event_id";

  r::ranking_response response;

  ds.choose_rank(event_id, JSON_CONTEXT_LEARNING, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
  size_t chosen_action;
  response.get_chosen_action_id(chosen_action, &status);
  BOOST_CHECK_EQUAL(chosen_action, 0);
  int current_expect_action_id = 0;
  for (auto it = response.begin(); it != response.end(); ++it) {
    BOOST_CHECK_EQUAL((*it).action_id, current_expect_action_id);
    current_expect_action_id++;
  }
}

BOOST_AUTO_TEST_CASE(live_model_request_continuous_action)
{
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  float min_value = 185;
  float max_value = 23959;
  size_t num_actions = 4;
  float bandwidth = 1;
  auto continuous_range = (max_value - min_value);
  auto unit_range = continuous_range / float(num_actions);

  std::string cmd = "--cats " + std::to_string(num_actions) +
   " --min_value " + std::to_string(min_value) + " --max_value " + std::to_string(max_value) + " --bandwidth " + std::to_string(bandwidth) +
   " --coin --loss_option 1 --json --quiet --epsilon 0.1 --id N/A";
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, cmd.c_str());
  // only added for version 2
  config.set(r::name::PROTOCOL_VERSION, "2");

  r::api_status status;

  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::continuous_action_response response;

  BOOST_CHECK_EQUAL(ds.request_continuous_action(JSON_CONTEXT_CONTINUOUS_ACTIONS, response, &status), err::success);
  // expected to fall in first unit range if no model exists
  BOOST_CHECK_GE(response.get_chosen_action(), min_value);
  BOOST_CHECK_LE(response.get_chosen_action(), min_value + unit_range);
  // pdf_value on explore ~= (1 - e) * (1.0 / 2 * b) + e / (continuous_range)
  // pdf_value on exploit = e / (continuous_range)
  BOOST_CHECK_CLOSE(response.get_chosen_action_pdf_value(), 4.20627566e-06, FLOAT_TOL);

  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
}

BOOST_AUTO_TEST_CASE(live_model_request_continuous_action_invalid_ctx)
{
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  float min_value = 185;
  float max_value = 23959;
  size_t num_actions = 4;
  float bandwidth = 1;
  auto continuous_range = (max_value - min_value);
  auto unit_range = continuous_range / float(num_actions);

  std::string cmd = "--cats " + std::to_string(num_actions) +
   " --min_value " + std::to_string(min_value) + " --max_value " + std::to_string(max_value) + " --bandwidth " + std::to_string(bandwidth) +
   " --coin --loss_option 1 --json --quiet --epsilon 0.1 --id N/A";
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, cmd.c_str());
  // only added for version 2
  config.set(r::name::PROTOCOL_VERSION, "2");

  r::api_status status;

  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::continuous_action_response response;

  const auto invalid_context = "";
  BOOST_CHECK_EQUAL(ds.request_continuous_action(invalid_context, response, &status), err::invalid_argument); // invalid context
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);
}

BOOST_AUTO_TEST_CASE(live_model_request_decision) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;

  // Create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::decision_response response;

  // request ranking
  BOOST_CHECK_EQUAL(ds.request_decision(JSON_CONTEXT_WITH_SLOTS, response, &status), err::success);
  BOOST_CHECK_EQUAL(response.size(), 1);
  size_t chosen;
  BOOST_CHECK_EQUAL((*response.begin()).get_action_id(), 0);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  const auto invalid_context = "";
  BOOST_CHECK_EQUAL(ds.request_decision(invalid_context, response, &status), err::invalid_argument); // invalid context
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);

  const auto context_with_ids = R"({"GUser":{"hobby":"hiking","id":"a","major":"eng"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}],"_slots":[{"TSlot":{"s1":"f1"},"_id":"817985e8-74ac-415c-bb69-735099c94d4d"},{"TSlot":{"s2":"f2"},"_id":"afb1da57-d4cd-4691-97d8-2b24bfb4e07f"}]})";
  BOOST_CHECK_EQUAL(ds.request_decision(context_with_ids, response, &status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
  BOOST_CHECK_EQUAL(response.get_model_id(), "N/A");

  BOOST_CHECK_EQUAL(response.size(), 2);
  //check first slot
  auto it = response.begin();
  auto& resp = *it;
  BOOST_CHECK_EQUAL(resp.get_slot_id(), "817985e8-74ac-415c-bb69-735099c94d4d");
  BOOST_CHECK_EQUAL(resp.get_action_id(), 0);
  //check second slot
  ++it;
  auto& resp1 = *it;
  BOOST_CHECK_EQUAL(resp1.get_slot_id(), "afb1da57-d4cd-4691-97d8-2b24bfb4e07f");
  BOOST_CHECK_EQUAL(resp1.get_action_id(), 1);
}

BOOST_AUTO_TEST_CASE(live_model_ranking_request_pdf_passthrough) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::MODEL_IMPLEMENTATION, r::value::PASSTHROUGH_PDF_MODEL);

  // Background refresh introduces a timing issue where the model might not have updated properly
  // before the choose_rank() call.
  config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");

  r::api_status status;

  //create the ds live_model, and initialize it with the config

  r::live_model model = create_mock_live_model(config, &r::data_transport_factory, &r::model_factory, nullptr);

  BOOST_CHECK_EQUAL(model.init(&status), err::success);
  const auto event_id = "event_id";

  r::ranking_response response;

  // request ranking
  BOOST_CHECK_EQUAL(model.choose_rank(event_id, JSON_CONTEXT_PDF, response), err::success);

  size_t num_actions = response.size();
  BOOST_CHECK_EQUAL(num_actions, 2);

  // check that our PDF is what we expected
  r::ranking_response::iterator it = response.begin();
  const float* expected_probability = EXPECTED_PDF;

  for (uint32_t i = 0; i < num_actions; i++)
  {
    auto action_probability = *(it + i);
    BOOST_CHECK_EQUAL(action_probability.probability, EXPECTED_PDF[action_probability.action_id]);
  }
}

// Same with live_model_ranking_request_pdf_passthrough but using "_p"
BOOST_AUTO_TEST_CASE(live_model_ranking_request_pdf_passthrough_underscore_p) {
  // Create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::MODEL_IMPLEMENTATION, r::value::PASSTHROUGH_PDF_MODEL);

  // Background refresh introduces a timing issue where the model might not have updated properly before the choose_rank() call.
  config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");

  r::api_status status;

  // Create the ds live_model, and initialize it with the config
  r::live_model model = create_mock_live_model(config, &r::data_transport_factory, &r::model_factory, nullptr);

  BOOST_CHECK_EQUAL(model.init(&status), err::success);
  const auto event_id = "event_id";

  r::ranking_response response;

  // Request ranking
  constexpr auto JSON_PDF = R"({"Shared":{"t":"abc"}, "_multi":[{"Action":{"c":1}},{"Action":{"c":2}}],"_p":[0.4, 0.6]})";
  BOOST_CHECK_EQUAL(model.choose_rank(event_id, JSON_PDF, response), err::success);

  size_t num_actions = response.size();
  BOOST_CHECK_EQUAL(num_actions, 2);

  // Check that our PDF is what we expected
  r::ranking_response::iterator it = response.begin();
  const float* expected_probability = EXPECTED_PDF;

  for (uint32_t i = 0; i < num_actions; i++)
  {
    auto action_probability = *(it + i);
    BOOST_CHECK_EQUAL(action_probability.probability, EXPECTED_PDF[action_probability.action_id]);
  }
}

BOOST_AUTO_TEST_CASE(live_model_outcome) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  //create a ds live_model, and initialize with configuration
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);

  //check api_status content when errors are returned
  r::api_status status;

  BOOST_CHECK_EQUAL(ds.init(&status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  const auto event_id = "event_id";
  const auto  outcome = "outcome";
  const auto  invalid_event_id = "";
  const auto  invalid_outcome = "";

  // report outcome
  const auto scode = ds.report_outcome(event_id, outcome, &status);
  BOOST_CHECK_EQUAL(scode, err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  // check expected returned codes
  BOOST_CHECK_EQUAL(ds.report_outcome(invalid_event_id, outcome), err::invalid_argument); //invalid event_id
  BOOST_CHECK_EQUAL(ds.report_outcome(event_id, invalid_outcome), err::invalid_argument); //invalid outcome

  //invalid event_id
  ds.report_outcome(invalid_event_id, outcome, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), reinforcement_learning::error_code::invalid_argument);

  //invalid context
  ds.report_outcome(event_id, invalid_outcome, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), reinforcement_learning::error_code::invalid_argument);

  //valid request => status is not modified
  r::api_status::try_update(&status, -42, "hello");
  ds.report_outcome(event_id, outcome, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
}

BOOST_AUTO_TEST_CASE(live_model_outcome_with_secondary_id_and_v1) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  //create a ds live_model, and initialize with configuration
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);

  //check api_status content when errors are returned
  r::api_status status;

  BOOST_CHECK_EQUAL(ds.init(&status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  // report outcome
  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", 10, 1.5f, &status), err::protocol_not_supported);
  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", "valid_secondary", 1.5f, &status), err::protocol_not_supported);
  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", 10, "valid_outcome", &status), err::protocol_not_supported);
  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", "valid_secondary", "valid_outcome", &status), err::protocol_not_supported);
  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", "", 1.5f, &status), err::invalid_argument); //we fail the input check before we check for protocol versin
}


BOOST_AUTO_TEST_CASE(live_model_outcome_with_secondary_id_and_v2) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::PROTOCOL_VERSION, "2");

  //create a ds live_model, and initialize with configuration
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);

  //check api_status content when errors are returned
  r::api_status status;

  BOOST_CHECK_EQUAL(ds.init(&status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  // report outcome
  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", 10, 1.5f, &status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", "valid_secondary", 1.5f, &status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");


  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", 10, "valid_outcome", &status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", "valid_secondary", "valid_outcome", &status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  // check expected returned codes
  BOOST_CHECK_EQUAL(ds.report_outcome("event_id", "", 1.5f), err::invalid_argument);
}

namespace r = reinforcement_learning;

class wrong_class {};

class algo_server {
public:
  algo_server() : _err_count{ 0 } {}
  void ml_error_handler(void) { shutdown(); }
  int _err_count;
private:
  void shutdown() { ++_err_count; }
};

void algo_error_func(const r::api_status&, algo_server* ph) {
  ph->ml_error_handler();
}

BOOST_AUTO_TEST_CASE(typesafe_err_callback) {
  //start a http server that will receive events sent from the eventhub_client
  auto mock_sender = get_mock_sender(r::error_code::http_bad_status_code);
  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model(r::model_management::model_type_t::CB);

  auto sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get());
  auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  auto model_factory = get_mock_model_factory(mock_model.get());

  //create a simple ds configuration
  u::configuration config;
  auto const status = cfg::create_from_json(JSON_CFG, config);
  BOOST_CHECK_EQUAL(status, r::error_code::success);
  config.set(r::name::EH_TEST, "true");


  ////////////////////////////////////////////////////////////////////
  //// Following mismatched object type is prevented by the compiler
  //   wrong_class mismatch;
  //   live_model ds2(config, algo_error_func, &mismatch);
  ////////////////////////////////////////////////////////////////////

  algo_server the_server;

  // Create live_model in own scope so that destructor can be forced, flushing buffers and queues.
  {
    //create a ds live_model, and initialize with configuration
    r::live_model ds(config, algo_error_func, &the_server, &r::trace_logger_factory, data_transport_factory.get(), model_factory.get(), sender_factory.get());

    ds.init(nullptr);

    const char* event_id = "event_id";

    r::ranking_response response;
    BOOST_CHECK_EQUAL(the_server._err_count, 0);
    // request ranking
    BOOST_CHECK_EQUAL(ds.choose_rank(event_id, JSON_CONTEXT, response), r::error_code::success);
  }

  BOOST_CHECK_GT(the_server._err_count, 0);
}

BOOST_AUTO_TEST_CASE(live_model_mocks) {
  std::vector<buffer_data_t> recorded;
  auto mock_sender = get_mock_sender(recorded);
  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model(r::model_management::model_type_t::CB);

  auto sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get());
  auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  auto model_factory = get_mock_model_factory(mock_model.get());

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  {
    r::live_model model = create_mock_live_model(config, data_transport_factory.get(), model_factory.get(), sender_factory.get());

    r::api_status status;
    BOOST_CHECK_EQUAL(model.init(&status), err::success);

    const auto event_id = "event_id";
    r::ranking_response response;

    BOOST_CHECK_EQUAL(model.choose_rank(event_id, JSON_CONTEXT, response), err::success);
    BOOST_CHECK_EQUAL(model.report_outcome(event_id, 1.0), err::success);

    Verify(Method((*mock_sender), init)).Exactly(2);
  }
  BOOST_CHECK_EQUAL(recorded.size(), 2);
}

BOOST_AUTO_TEST_CASE(live_model_background_refresh) {
    u::configuration config;
    cfg::create_from_json(JSON_CFG, config);

    config.set(r::name::EH_TEST, "true");

    r::live_model model = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);

    r::api_status status;
    BOOST_CHECK_EQUAL(model.init(&status), err::success);
    BOOST_CHECK_EQUAL(model.refresh_model(&status), err::model_update_error);
}

BOOST_AUTO_TEST_CASE(live_model_no_background_refresh) {
    u::configuration config;
    cfg::create_from_json(JSON_CFG, config);

    config.set(r::name::EH_TEST, "true");
    config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");

    r::live_model model = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);

    r::api_status status;
    BOOST_CHECK_EQUAL(model.init(&status), err::success);
    BOOST_CHECK_EQUAL(model.refresh_model(&status), err::success);
}

BOOST_AUTO_TEST_CASE(live_model_no_background_refresh_failure) {
    u::configuration config;
    cfg::create_from_json(JSON_CFG, config);

    config.set(r::name::EH_TEST, "true");
    config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");

    auto mock_data_transport = get_mock_failing_data_transport();
    auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());

    r::live_model model = create_mock_live_model(config, data_transport_factory.get());

    r::api_status status;
    BOOST_CHECK_NE(model.init(&status), err::success);
}

BOOST_AUTO_TEST_CASE(live_model_logger_receive_data) {
  std::vector<buffer_data_t> recorded_observations;
  auto mock_observation_sender = get_mock_sender(recorded_observations);

  std::vector<buffer_data_t> recorded_interactions;
  auto mock_interaction_sender = get_mock_sender(recorded_interactions);

  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model(r::model_management::model_type_t::CB);

  auto logger_factory = get_mock_sender_factory(mock_observation_sender.get(), mock_interaction_sender.get());
  auto data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  auto model_factory = get_mock_model_factory(mock_model.get());

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  auto const version_number = "1";

  auto const event_id_1 = "event_id";
  auto const event_id_2 = "event_id_2";

  auto const expected_interaction_1 = u::concat(R"({"Version":")", version_number, R"(","EventId":")", event_id_1, R"(","a":[1,2],"c":)", JSON_CONTEXT, R"(,"p":[0.500000,0.500000],"VWState":{"m":"N/A"}})");
  auto const expected_observation_1 = u::concat(R"({"EventId":")", event_id_1, R"(","v":1.000000})");

  auto const expected_interaction_2 = u::concat(R"({"Version":")", version_number, R"(","EventId":")", event_id_2, R"(","a":[1,2],"c":)", JSON_CONTEXT, R"(,"p":[0.500000,0.500000],"VWState":{"m":"N/A"}})");
  auto const expected_observation_2 = u::concat(R"({"EventId":")", event_id_2, R"(","v":1.000000})");
  auto const num_iterations = 5;

  std::string expected_interactions;
  std::string expected_observations;
  {
    r::live_model model = create_mock_live_model(config, data_transport_factory.get(), model_factory.get(), logger_factory.get());

    r::api_status status;
    BOOST_CHECK_EQUAL(model.init(&status), err::success);

    r::ranking_response response;
    for (auto i = 0; i < num_iterations; i++) {
      BOOST_CHECK_EQUAL(model.choose_rank(event_id_1, JSON_CONTEXT, response), err::success);
      BOOST_CHECK_EQUAL(model.report_outcome(event_id_1, 1.0), err::success);
      expected_interactions += expected_interaction_1 + '\n';
      expected_observations += expected_observation_1 + '\n';

      BOOST_CHECK_EQUAL(model.choose_rank(event_id_2, JSON_CONTEXT, response), err::success);
      BOOST_CHECK_EQUAL(model.report_outcome(event_id_2, 1.0), err::success);
      expected_interactions += expected_interaction_2;
      expected_observations += expected_observation_2;
      if (i + 1 < num_iterations) {
        expected_interactions += '\n';
        expected_observations += '\n';
      }
    }

    Verify(Method((*mock_observation_sender), init)).Exactly(1);
    Verify(Method((*mock_interaction_sender), init)).Exactly(1);
  }
  // TODO deserialize and check contents with expected
  BOOST_CHECK_GE(recorded_interactions.size(), 1);
  BOOST_CHECK_GE(recorded_observations.size(), 1);
}

BOOST_AUTO_TEST_CASE(populate_response_same_size_test) {
    r::api_status status;
    std::vector<std::vector<uint32_t>> action_ids = {{0,1,2}, {1,2}, {2}};
    std::vector<std::vector<float>> pdfs = {{0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f}};
    std::vector<const char*> event_ids = {"a", "b", "c"};
    std::string model_id = "id";

    r::decision_response resp;

    BOOST_CHECK_EQUAL(populate_response(action_ids, pdfs, event_ids, std::move(model_id), resp, nullptr, &status), err::success);

    BOOST_CHECK(strcmp(resp.get_model_id(), "id") == 0);
    BOOST_CHECK_EQUAL(resp.size(), 3);

    auto it = resp.begin();
    size_t action_id = 0;
    auto& rank_resp = *it;
    BOOST_CHECK(strcmp(rank_resp.get_slot_id(), "a") == 0);
    BOOST_CHECK_EQUAL(rank_resp.get_action_id(), 0);
    BOOST_CHECK_CLOSE(rank_resp.get_probability(), 0.8667f, FLOAT_TOL);
    ++it;

    auto& rank_resp1 = *it;
    BOOST_CHECK(strcmp(rank_resp1.get_slot_id(), "b") == 0);
    BOOST_CHECK_EQUAL(rank_resp1.get_action_id(), 1);
    BOOST_CHECK_CLOSE(rank_resp1.get_probability(), 0.9f, FLOAT_TOL);
    ++it;

    auto& rank_resp2 = *it;
    BOOST_CHECK(strcmp(rank_resp2.get_slot_id(), "c") == 0);
    BOOST_CHECK_EQUAL(rank_resp2.get_action_id(), 2);
    BOOST_CHECK_CLOSE(rank_resp2.get_probability(), 1.0f, FLOAT_TOL);
}

BOOST_AUTO_TEST_CASE(populate_response_different_size_test) {
    r::api_status status;
    std::vector<std::vector<uint32_t>> action_ids = {{0,1,2}, {1,2}};
    std::vector<std::vector<float>> pdfs = {{0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f}};
    std::vector<const char*> event_ids = {"a", "b", "c"};
    std::string model_id = "id";

    r::decision_response resp;

    BOOST_CHECK_EQUAL(populate_response(action_ids, pdfs, event_ids, std::move(model_id), resp, nullptr, &status), err::invalid_argument);

    action_ids = {{0,1}, {1,2}, {1}};
    pdfs = {{0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f}};
    BOOST_CHECK_EQUAL(populate_response(action_ids, pdfs, event_ids, std::move(model_id), resp, nullptr, &status), err::invalid_argument);
}

BOOST_AUTO_TEST_CASE(populate_slot_test) {
  r::api_status status;
  const std::vector<uint32_t> action_ids = { 0, 1, 2 };
  const std::vector<float> pdfs = { 0.8667f, 0.9f, 1.0f };
  const std::string slot_id = "testslot";

  r::slot_ranking slot;

  BOOST_CHECK_EQUAL(populate_slot(action_ids, pdfs, slot, slot_id, nullptr, &status), err::success);

  BOOST_CHECK_EQUAL(slot.size(), 3);

  size_t action_id;
  int x = slot.get_chosen_action_id(action_id);
  BOOST_CHECK_EQUAL(action_id, 0);

  int i = 0;
  for (const auto& d : slot) {
    BOOST_CHECK_EQUAL(d.action_id, action_ids[i]);
    BOOST_CHECK_EQUAL(d.probability, pdfs[i++]);
  }
}

BOOST_AUTO_TEST_CASE(populate_multi_slot_response_same_size_test) {
  r::api_status status;
  std::vector<std::vector<uint32_t>> action_ids = { {0,1,2}, {1,2}, {2} };
  std::vector<std::vector<float>> pdfs = { {0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f} };
  std::vector<std::string> slot_ids = { "slot0", "slot1", "slot2" };
  std::string event_id = "a";
  std::string model_id = "id";

  r::multi_slot_response_detailed resp;
  resp.resize(3);

  BOOST_CHECK_EQUAL(populate_multi_slot_response_detailed(action_ids, pdfs, std::move(event_id), std::move(model_id), slot_ids, resp, nullptr, &status), err::success);

  BOOST_CHECK(strcmp(resp.get_model_id(), "id") == 0);
  BOOST_CHECK_EQUAL(resp.size(), 3);

  auto it = resp.begin();
  size_t action_id = 0;
  auto& rank_resp = *it;

  int i = 0;
  for (const auto& s : resp) {
    int j = 0;
    for (const auto& d : s) {
      BOOST_CHECK_EQUAL(d.action_id, action_ids[i][j]);
      BOOST_CHECK_EQUAL(d.probability, pdfs[i][j++]);
    }
    ++i;
  }
}

BOOST_AUTO_TEST_CASE(populate_multi_slot_response_different_size_test) {
  r::api_status status;
  std::vector<std::vector<uint32_t>> action_ids = { {0,1,2}, {1,2} };
  std::vector<std::vector<float>> pdfs = { {0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f} };
  std::vector<std::string> slot_ids = { "slot0", "slot1", "slot2" };
  std::string event_id = "a";
  std::string model_id = "id";

  r::multi_slot_response_detailed resp;

  BOOST_CHECK_EQUAL(populate_multi_slot_response_detailed(action_ids, pdfs, std::move(event_id), std::move(model_id), slot_ids, resp, nullptr, &status), err::invalid_argument);

  action_ids = { {0,1}, {1,2}, {1} };
  pdfs = { {0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f} };
  BOOST_CHECK_EQUAL(populate_multi_slot_response_detailed(action_ids, pdfs, std::move(event_id), std::move(model_id), slot_ids, resp, nullptr, &status), err::invalid_argument);
}



const auto JSON_CCB_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[ { "TAction":{"a1":"f1"} },{"TAction":{"a2":"f2"}}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a1":"f1"}}]})";

BOOST_AUTO_TEST_CASE(ccb_explore_only_mode) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME,"interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME,"observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::decision_response response;
  BOOST_CHECK_EQUAL(model.request_decision(JSON_CCB_CONTEXT, response), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  size_t action_id = 0;
  auto& rank_resp = *it;
  BOOST_CHECK(strcmp(rank_resp.get_slot_id(), "") != 0);
  BOOST_CHECK_EQUAL(rank_resp.get_action_id(), 0);
  BOOST_CHECK_CLOSE(rank_resp.get_probability(), 1.f, FLOAT_TOL);
  ++it;

  auto& rank_resp1 = *it;
  BOOST_CHECK(strcmp(rank_resp1.get_slot_id(), "") != 0);
  BOOST_CHECK_EQUAL(rank_resp1.get_action_id(), 1);
  BOOST_CHECK_CLOSE(rank_resp1.get_probability(), 1.f, FLOAT_TOL);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(JSON_CCB_CONTEXT, response), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);


  auto it = response.begin();
  auto& slot_entry0 = *it;
  size_t action_id = slot_entry0.get_action_id();
  BOOST_CHECK(strcmp(slot_entry0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 0);
  ++it;

  auto& slot_entry1 = *it;
  action_id = slot_entry1.get_action_id();
  BOOST_CHECK(strcmp(slot_entry1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_apprentice_mode_no_baseline) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_APPRENTICE);

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(JSON_CCB_CONTEXT, response), err::baseline_actions_not_defined);
}

BOOST_AUTO_TEST_CASE(multi_slot_response_apprentice_mode_with_baseline) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_APPRENTICE);

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  int baseline_actions[2] = { 3, 1 };
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(nullptr, JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response,  baseline_actions, 2), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  auto& slot_entry0 = *it;
  size_t action_id = slot_entry0.get_action_id();
  BOOST_CHECK(strcmp(slot_entry0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 3);
  BOOST_CHECK_EQUAL(slot_entry0.get_probability(), 1.0);
  ++it;

  auto& slot_entry1 = *it;
  action_id = slot_entry1.get_action_id();
  BOOST_CHECK(strcmp(slot_entry1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  BOOST_CHECK_EQUAL(slot_entry0.get_probability(), 1.0);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_logging_only_with_baseline) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_LOGGINGONLY);

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  //baseline actions are not valid action indicies, this is for testing purposes to make sure the action id is overriden in apprentice / logginonly mode.
  int baseline_actions[2] = { 3, 1 };
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(nullptr, JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, baseline_actions, 2), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  auto& slot_entry0 = *it;
  size_t action_id = slot_entry0.get_action_id();
  BOOST_CHECK(strcmp(slot_entry0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 3);
  BOOST_CHECK_EQUAL(slot_entry0.get_probability(), 1.0);
  ++it;

  auto& slot_entry1 = *it;
  action_id = slot_entry1.get_action_id();
  BOOST_CHECK(strcmp(slot_entry1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  BOOST_CHECK_EQUAL(slot_entry0.get_probability(), 1.0);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_logging_only_no_baseline) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --id N/A");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_LOGGINGONLY);

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(nullptr, JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, nullptr, 0), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  auto& slot_entry0 = *it;
  size_t action_id = slot_entry0.get_action_id();
  BOOST_CHECK(strcmp(slot_entry0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 0);
  BOOST_CHECK_EQUAL(slot_entry0.get_probability(), 1.0);
  ++it;

  auto& slot_entry1 = *it;
  action_id = slot_entry1.get_action_id();
  BOOST_CHECK(strcmp(slot_entry1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  BOOST_CHECK_EQUAL(slot_entry0.get_probability(), 1.0);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_with_baseline_null_eventId) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  int baseline_actions[2] = { 3, 1 };
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(nullptr, JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, baseline_actions, 2), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);


  auto it = response.begin();
  auto& slot_entry0 = *it;
  size_t action_id = slot_entry0.get_action_id();
  BOOST_CHECK(strcmp(slot_entry0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 0);
  ++it;

  auto& slot_entry1 = *it;
  action_id = slot_entry1.get_action_id();
  BOOST_CHECK(strcmp(slot_entry1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_with_baseline_custom_eventId) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  int baseline_actions[2] = { 3, 1 };
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision("testEventId", JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, baseline_actions, 2), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);
  BOOST_CHECK_EQUAL(response.get_event_id(), "testEventId");


  auto it = response.begin();
  auto& slot_entry0 = *it;
  size_t action_id = slot_entry0.get_action_id();
  BOOST_CHECK(strcmp(slot_entry0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 0);
  ++it;

  auto& slot_entry1 = *it;
  action_id = slot_entry1.get_action_id();
  BOOST_CHECK(strcmp(slot_entry1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response_detailed response;
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(JSON_CCB_CONTEXT, response), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);


  auto it = response.begin();
  size_t action_id = 0;
  auto& slot_rank0 = *it;
  slot_rank0.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 0);
  ++it;

  auto& slot_rank1 = *it;
  slot_rank1.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_apprentice_mode_no_baseline) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_APPRENTICE);

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response_detailed response;
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(JSON_CCB_CONTEXT, response), err::baseline_actions_not_defined);
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_apprentice_mode_with_baseline) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_APPRENTICE);

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response_detailed response;
  int baseline_actions[2] = { 3, 1 };
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(nullptr, JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, baseline_actions, 2), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  size_t action_id = 0;
  auto& slot_rank0 = *it;
  slot_rank0.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 3);
  ++it;

  auto& slot_rank1 = *it;
  slot_rank1.get_chosen_action_id(action_id);
  slot_rank1.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_logging_only_with_baseline) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_LOGGINGONLY);

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response_detailed response;
  int baseline_actions[2] = { 3, 1 };
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(nullptr, JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, baseline_actions, 2), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  size_t action_id = 0;
  auto& slot_rank0 = *it;
  slot_rank0.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 3);
  ++it;

  auto& slot_rank1 = *it;
  slot_rank1.get_chosen_action_id(action_id);
  slot_rank1.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_logging_only_no_baseline) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --id N/A");
  config.set(r::name::LEARNING_MODE, r::value::LEARNING_MODE_LOGGINGONLY);

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response_detailed response;
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(nullptr, JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, nullptr, 0), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  size_t action_id = 0;
  auto& slot_rank0 = *it;
  slot_rank0.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 0);
  ++it;

  auto& slot_rank1 = *it;
  slot_rank1.get_chosen_action_id(action_id);
  slot_rank1.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_with_baseline_null_eventid) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response_detailed response;
  int baseline_actions[2] = { 3, 1 };
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(nullptr, JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, baseline_actions, 2), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  size_t action_id = 0;
  auto& slot_rank0 = *it;
  slot_rank0.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 0);
  ++it;

  auto& slot_rank1 = *it;
  slot_rank1.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_with_baseline_custom_eventid) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response_detailed response;
  int baseline_actions[2] = { 3, 1 };
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision("testEventId", JSON_CCB_CONTEXT, r::action_flags::DEFAULT, response, baseline_actions, 2), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);
  BOOST_CHECK_EQUAL(response.get_event_id(), "testEventId");


  auto it = response.begin();
  size_t action_id = 0;
  auto& slot_rank0 = *it;
  slot_rank0.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank0.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 0);
  ++it;

  auto& slot_rank1 = *it;
  slot_rank1.get_chosen_action_id(action_id);
  BOOST_CHECK(strcmp(slot_rank1.get_id(), "") != 0);
  BOOST_CHECK_EQUAL(action_id, 1);
  ++it;
}

const auto JSON_SLATES_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"},"_slot_id":0},{"TAction":{"a2":"f2"},"_slot_id":0},{"TAction":{"a3":"f3"},"_slot_id":1},{"TAction":{"a4":"f4"},"_slot_id":1},{"TAction":{"a5":"f5"},"_slot_id":1}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a2":"f2"}}]})";

BOOST_AUTO_TEST_CASE(slates_explore_only_mode) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME,"interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME,"observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(JSON_SLATES_CONTEXT, response), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  size_t action_id = 0;
  auto& slot_response = *it;
  std::string first_slot_id = slot_response.get_id();
  BOOST_CHECK_NE(slot_response.get_id(), ""); //uuid
  BOOST_CHECK_EQUAL(slot_response.get_action_id(), 0);
  BOOST_CHECK_CLOSE(slot_response.get_probability(), 1.f, FLOAT_TOL);
  ++it;

  auto& slot_response1 = *it;
  BOOST_CHECK_NE(slot_response1.get_id(), ""); //uuid
  BOOST_CHECK_NE(slot_response1.get_id(), first_slot_id); //uuid
  BOOST_CHECK_EQUAL(slot_response1.get_action_id(), 0);
  BOOST_CHECK_CLOSE(slot_response1.get_probability(), 1.f, FLOAT_TOL);
  ++it;

  BOOST_CHECK(it == response.end());
}

BOOST_AUTO_TEST_CASE(live_model_ccb_and_v2) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;

  // Create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr, r::model_management::model_type_t::CCB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::multi_slot_response slates_response;
  r::decision_response ccb_response;

  //slates API doesn't work for ccb under v1 protocol
  BOOST_CHECK_EQUAL(ds.request_multi_slot_decision("event_id", JSON_CONTEXT_WITH_SLOTS, slates_response, &status), err::protocol_not_supported);

  config.set(r::name::PROTOCOL_VERSION, "2");
  r::live_model ds2 = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr, r::model_management::model_type_t::CCB);
  BOOST_CHECK_EQUAL(ds2.init(&status), err::success);

  //slates API work for ccb with v2
  BOOST_CHECK_EQUAL(ds2.request_multi_slot_decision("event_id", JSON_CONTEXT_WITH_SLOTS, slates_response, &status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  //old ccb api doesn't work under v2
  BOOST_CHECK_EQUAL(ds2.request_decision(JSON_CONTEXT_WITH_SLOTS, ccb_response, &status), err::protocol_not_supported);

}

BOOST_AUTO_TEST_CASE(live_model_ccb_and_v2_w_slot_ids) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  config.set(r::name::PROTOCOL_VERSION, "2");

  r::api_status status;

  // Create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr, r::model_management::model_type_t::CCB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::multi_slot_response response;

  BOOST_CHECK_EQUAL(ds.request_multi_slot_decision("event_id", JSON_CONTEXT_WITH_SLOTS_WITH_SLOT_IDS, response, &status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  auto it = response.begin();
  auto& ccb_response = *it;
  BOOST_CHECK_EQUAL(ccb_response.get_id(), "provided_slot_id_1");
  ++it;

  auto& ccb_response2 = *it;
  BOOST_CHECK_NE(ccb_response2.get_id(), "provided_slot_id_1");
  BOOST_CHECK_NE(ccb_response2.get_id(), ""); //uuid
  ++it;
  BOOST_CHECK(it == response.end());
}


BOOST_AUTO_TEST_CASE(live_model_ccb_and_v2_w_slot_ids_and_slot_ns) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --chain_hash --json --quiet --epsilon 0.0 --first_only --id N/A");
  config.set(r::name::PROTOCOL_VERSION, "2");

  r::api_status status;

  // Create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr, r::model_management::model_type_t::CCB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::multi_slot_response response;

  BOOST_CHECK_EQUAL(ds.request_multi_slot_decision("event_id", JSON_CONTEXT_WITH_SLOTS_W_SLOT_IDS_AND_SLOT_NS, response, &status), err::success);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  auto it = response.begin();
  auto& ccb_response = *it;
  BOOST_CHECK_EQUAL(ccb_response.get_id(), "provided_slot_id_1");
  ++it;

  auto& ccb_response2 = *it;
  auto uuid_response_2 = ccb_response2.get_id();
  BOOST_CHECK_NE(ccb_response2.get_id(), "provided_slot_id_1");
  BOOST_CHECK_NE(uuid_response_2, ""); //uuid
  ++it;

  auto& ccb_response_3 = *it;
  auto uuid_response_3 = ccb_response_3.get_id();
  BOOST_CHECK_NE(ccb_response_3.get_id(), "provided_slot_id_1");
  BOOST_CHECK_NE(uuid_response_3, ""); //uuid
  BOOST_CHECK_NE(uuid_response_3, uuid_response_2); //uuid

  ++it;
  BOOST_CHECK(it == response.end());
}

#ifdef USE_AZURE_FACTORIES
BOOST_AUTO_TEST_CASE(live_model_using_endpoint_success) {
    u::configuration config;
    cfg::create_from_json(JSON_CFG_API, config);
    config.set("http.api.key", "apiKey1234");
    config.set("model.blob.uri", "http://localhost:8080/personalizer/v1.1-preview.1/model");
    r::api_status status;
    std::unique_ptr<reinforcement_learning::live_model> _rl = std::unique_ptr<r::live_model>(new r::live_model(config, nullptr));

    BOOST_CHECK_EQUAL(_rl->init(&status), err::success);
}

BOOST_AUTO_TEST_CASE(live_model_using_endpoint_failure_no_uri) {
    u::configuration config;
    cfg::create_from_json(JSON_CFG_API, config);
    config.set("http.api.key", "Bearer apiKey1234");
    config.set("http.api.header.key.name" , "Authorization");
    r::api_status status;
    std::unique_ptr<reinforcement_learning::live_model> _rl = std::unique_ptr<r::live_model>(new r::live_model(config, nullptr));

    BOOST_CHECK_EQUAL(_rl->init(&status), r::error_code::http_model_uri_not_provided);
}

BOOST_AUTO_TEST_CASE(live_model_using_endpoint_failure_no_apikey) {
    u::configuration config;
    cfg::create_from_json(JSON_CFG_API, config);
    config.set("model.blob.uri", "http://localhost:8080/personalizer/v1.1-preview.1/model");
    r::api_status status;
    std::unique_ptr<reinforcement_learning::live_model> _rl = std::unique_ptr<r::live_model>(new r::live_model(config, nullptr));

    BOOST_CHECK_EQUAL(_rl->init(&status), r::error_code::http_api_key_not_provided);
}
#endif
