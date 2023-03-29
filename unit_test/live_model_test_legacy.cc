#include "multi_slot_response.h"
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "api_status.h"
#include "config_utility.h"
#include "constants.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "live_model.h"
#include "mock_util.h"
#include "model_mgmt.h"
#include "ranking_response.h"
#include "sampling.h"
#include "sender.h"
#include "str_util.h"

#include <fstream>
#include <thread>
#include <vector>

constexpr float FLOAT_TOL = 0.0001f;
#ifdef __GNUG__

// Fakeit does not work with GCC's devirtualization
// which is enabled with -O2 (the default) or higher.
#  pragma GCC optimize("no-devirtualize")

#endif

#include <fakeit/fakeit.hpp>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace m = reinforcement_learning::model_management;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

using namespace fakeit;

namespace
{
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
const auto JSON_CONTEXT_WITH_SLOTS_W_SLOT_IDS_AND_SLOT_NS =
    R"({"_multi":[{},{}, {}],"_slots":[{"_id":"provided_slot_id_1", "slot_namespace":{"a":"b"}}, {}, {"ns":{"_id":"ignored_slot_id", "a":"b"}}]})";
const auto JSON_CONTEXT_PDF =
    R"({"Shared":{"t":"abc"}, "_multi":[{"Action":{"c":1}},{"Action":{"c":2}}],"p":[0.4, 0.6]})";
const auto JSON_CONTEXT_LEARNING =
    R"({"Shared":{"t":"abc"}, "_multi":[{"Action":{"c":1}},{"Action":{"c":2}},{"Action":{"c":3}}],"p":[0.4, 0.1, 0.5]})";
const auto JSON_CONTEXT_CONTINUOUS_ACTIONS =
    R"({"Temperature":{"18-25":1,"4":1,"C":1,"0":1,"1":1,"2":1,"15":1,"M":1}})";
const float EXPECTED_PDF[2] = {0.4f, 0.6f};

r::live_model create_mock_live_model(const u::configuration& config,
    r::data_transport_factory_t* data_transport_factory = nullptr, r::model_factory_t* model_factory = nullptr,
    r::sender_factory_t* sender_factory = nullptr,
    r::model_management::model_type_t model_type = r::model_management::model_type_t::UNKNOWN)
{
  static auto mock_sender = get_mock_sender(r::error_code::success);
  static auto mock_data_transport = get_mock_data_transport();
  static auto mock_model = get_mock_model(model_type);

  static auto default_sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get());
  static auto default_data_transport_factory = get_mock_data_transport_factory(mock_data_transport.get());
  static auto default_model_factory = get_mock_model_factory(mock_model.get());

  if (!data_transport_factory) { data_transport_factory = default_data_transport_factory.get(); }

  if (!model_factory) { model_factory = default_model_factory.get(); }

  if (!sender_factory) { sender_factory = default_sender_factory.get(); }

  r::live_model model(
      config, nullptr, nullptr, &r::trace_logger_factory, data_transport_factory, model_factory, sender_factory);
  return model;
}

}  // namespace

BOOST_AUTO_TEST_CASE(live_model_ranking_request_legacy)
{
  // create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  r::api_status status;

  // create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, nullptr, nullptr, r::model_management::model_type_t::CB);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  const auto event_id = "event_id";
  const auto invalid_event_id = "";
  const auto invalid_context = "";

  r::ranking_response response;

  // request ranking
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, JSON_CONTEXT, response), err::success);

  // check expected returned codes
  BOOST_CHECK_EQUAL(
      ds.choose_rank(invalid_event_id, JSON_CONTEXT, response), err::invalid_argument);           // invalid event_id
  BOOST_CHECK_EQUAL(ds.choose_rank(event_id, invalid_context, response), err::invalid_argument);  // invalid context

  // invalid event_id
  ds.choose_rank(event_id, invalid_context, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);

  // invalid context
  ds.choose_rank(invalid_event_id, JSON_CONTEXT, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);

  // valid request => status is reset
  r::api_status::try_update(&status, -42, "hello");
  ds.choose_rank(event_id, JSON_CONTEXT, response, &status);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
}

BOOST_AUTO_TEST_CASE(live_model_request_continuous_action_legacy)
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

  std::string cmd = "--cats " + std::to_string(num_actions) + " --min_value " + std::to_string(min_value) +
      " --max_value " + std::to_string(max_value) + " --bandwidth " + std::to_string(bandwidth) +
      " --coin --loss_option 1 --json --quiet --epsilon 0.1 --id N/A";
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, cmd.c_str());
  // only added for version 2
  config.set(r::name::PROTOCOL_VERSION, "2");

  r::api_status status;

  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::continuous_action_response response;

  RL_IGNORE_DEPRECATED_USAGE_START
  BOOST_CHECK_EQUAL(ds.request_continuous_action(JSON_CONTEXT_CONTINUOUS_ACTIONS, response, &status), err::success);
  RL_IGNORE_DEPRECATED_USAGE_END
  // expected to fall in first unit range if no model exists
  BOOST_CHECK_GE(response.get_chosen_action(), min_value);
  BOOST_CHECK_LE(response.get_chosen_action(), min_value + unit_range);
  // pdf_value on explore ~= (1 - e) * (1.0 / 2 * b) + e / (continuous_range)
  // pdf_value on exploit = e / (continuous_range)
  BOOST_CHECK_CLOSE(response.get_chosen_action_pdf_value(), 4.20627566e-06, FLOAT_TOL);

  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
}

BOOST_AUTO_TEST_CASE(live_model_request_decision_legacy)
{
  // create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(
      r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;

  // Create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::decision_response response;

  // request ranking
  RL_IGNORE_DEPRECATED_USAGE_START
  BOOST_CHECK_EQUAL(ds.request_decision(JSON_CONTEXT_WITH_SLOTS, response, &status), err::success);
  RL_IGNORE_DEPRECATED_USAGE_END
  BOOST_CHECK_EQUAL(response.size(), 1);
  BOOST_CHECK_EQUAL((*response.begin()).get_action_id(), 0);
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");

  const auto invalid_context = "";
  RL_IGNORE_DEPRECATED_USAGE_START
  BOOST_CHECK_EQUAL(ds.request_decision(invalid_context, response, &status), err::invalid_argument);  // invalid context
  RL_IGNORE_DEPRECATED_USAGE_END
  BOOST_CHECK_EQUAL(status.get_error_code(), err::invalid_argument);

  const auto context_with_ids =
      R"({"GUser":{"hobby":"hiking","id":"a","major":"eng"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}],"_slots":[{"TSlot":{"s1":"f1"},"_id":"817985e8-74ac-415c-bb69-735099c94d4d"},{"TSlot":{"s2":"f2"},"_id":"afb1da57-d4cd-4691-97d8-2b24bfb4e07f"}]})";
  RL_IGNORE_DEPRECATED_USAGE_START
  BOOST_CHECK_EQUAL(ds.request_decision(context_with_ids, response, &status), err::success);
  RL_IGNORE_DEPRECATED_USAGE_END
  BOOST_CHECK_EQUAL(status.get_error_code(), 0);
  BOOST_CHECK_EQUAL(status.get_error_msg(), "");
  BOOST_CHECK_EQUAL(response.get_model_id(), "N/A");

  BOOST_CHECK_EQUAL(response.size(), 2);
  // check first slot
  auto it = response.begin();
  auto& resp = *it;
  BOOST_CHECK_EQUAL(resp.get_slot_id(), "817985e8-74ac-415c-bb69-735099c94d4d");
  BOOST_CHECK_EQUAL(resp.get_action_id(), 0);
  // check second slot
  ++it;
  auto& resp1 = *it;
  BOOST_CHECK_EQUAL(resp1.get_slot_id(), "afb1da57-d4cd-4691-97d8-2b24bfb4e07f");
  BOOST_CHECK_EQUAL(resp1.get_action_id(), 1);
}

const auto JSON_SLATES_CONTEXT =
    R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"},"_slot_id":0},{"TAction":{"a2":"f2"},"_slot_id":0},{"TAction":{"a3":"f3"},"_slot_id":1},{"TAction":{"a4":"f4"},"_slot_id":1},{"TAction":{"a5":"f5"},"_slot_id":1}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a2":"f2"}}]})";

BOOST_AUTO_TEST_CASE(slates_explore_only_mode_legacy)
{
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME, "observation.txt");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE,
      "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::multi_slot_response response;
  RL_IGNORE_DEPRECATED_USAGE_START
  BOOST_CHECK_EQUAL(model.request_multi_slot_decision(JSON_SLATES_CONTEXT, response), err::success);
  RL_IGNORE_DEPRECATED_USAGE_END

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  size_t action_id = 0;
  auto& slot_response = *it;
  std::string first_slot_id = slot_response.get_id();
  BOOST_CHECK_NE(slot_response.get_id(), "");  // uuid
  BOOST_CHECK_EQUAL(slot_response.get_action_id(), 0);
  BOOST_CHECK_CLOSE(slot_response.get_probability(), 1.f, FLOAT_TOL);
  ++it;

  auto& slot_response1 = *it;
  BOOST_CHECK_NE(slot_response1.get_id(), "");             // uuid
  BOOST_CHECK_NE(slot_response1.get_id(), first_slot_id);  // uuid
  BOOST_CHECK_EQUAL(slot_response1.get_action_id(), 0);
  BOOST_CHECK_CLOSE(slot_response1.get_probability(), 1.f, FLOAT_TOL);
  ++it;

  BOOST_CHECK(it == response.end());
}
