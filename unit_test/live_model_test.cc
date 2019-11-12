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
#include "test_common.h"

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

  const auto JSON_CONTEXT = R"({"_multi":[{},{}]})";
  const auto JSON_CONTEXT_WITH_SLOTS = R"({"_multi":[{},{}],"_slots":[{}]})";
  const auto JSON_CONTEXT_PDF = R"({"Shared":{"t":"abc"}, "_multi":[{"Action":{"c":1}},{"Action":{"c":2}}],"p":[0.4, 0.6]})";
  const float EXPECTED_PDF[2] = { 0.4f, 0.6f };

  r::live_model create_mock_live_model(
    const u::configuration& config,
    r::data_transport_factory_t* data_transport_factory = nullptr,
    r::model_factory_t* model_factory = nullptr,
    r::sender_factory_t* sender_factory = nullptr) {

      static auto mock_sender = get_mock_sender(r::error_code::success);
      static auto mock_data_transport = get_mock_data_transport();
      static auto mock_model = get_mock_model();

      static auto default_sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get(), mock_sender.get());
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

BOOST_AUTO_TEST_CASE(live_model_ranking_request) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  r::api_status status;

  //create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config);
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

BOOST_AUTO_TEST_CASE(live_model_request_decision) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  r::api_status status;

  // Create the ds live_model, and initialize it with the config
  r::live_model ds = create_mock_live_model(config, nullptr, &reinforcement_learning::model_factory, nullptr);
  BOOST_CHECK_EQUAL(ds.init(&status), err::success);

  r::decision_response response;

  // request ranking
  BOOST_CHECK_EQUAL(ds.request_decision(JSON_CONTEXT_WITH_SLOTS, response, &status), err::success);
  BOOST_CHECK_EQUAL(response.size(), 1);
  size_t chosen;
  BOOST_CHECK_EQUAL((*response.begin()).get_chosen_action_id(chosen), err::success);
  BOOST_CHECK_EQUAL(chosen, 0);
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
  BOOST_CHECK_EQUAL(it->get_event_id(), "817985e8-74ac-415c-bb69-735099c94d4d");
  BOOST_CHECK_EQUAL(it->get_chosen_action_id(chosen), err::success);
  BOOST_CHECK_EQUAL(chosen, 0);
  //check second slot
  it++;
  BOOST_CHECK_EQUAL(it->get_event_id(), "afb1da57-d4cd-4691-97d8-2b24bfb4e07f");
  BOOST_CHECK_EQUAL(it->get_chosen_action_id(chosen), err::success);
  BOOST_CHECK_EQUAL(chosen, 1);
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

BOOST_AUTO_TEST_CASE(live_model_outcome) {
  //create a simple ds configuration
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");

  //create a ds live_model, and initialize with configuration
  r::live_model ds = create_mock_live_model(config);

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
  auto mock_model = get_mock_model();

  auto sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get(), mock_sender.get());
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
  std::vector<buffer_t> recorded;
  auto mock_sender = get_mock_sender(recorded);
  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model();

  auto sender_factory = get_mock_sender_factory(mock_sender.get(), mock_sender.get(), mock_sender.get());
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

    Verify(Method((*mock_sender), init)).Exactly(3);
  }
  BOOST_CHECK_EQUAL(recorded.size(), 2);
}

BOOST_AUTO_TEST_CASE(live_model_background_refresh) {
    u::configuration config;
    cfg::create_from_json(JSON_CFG, config);

    config.set(r::name::EH_TEST, "true");

    r::live_model model = create_mock_live_model(config);

    r::api_status status;
    BOOST_CHECK_EQUAL(model.init(&status), err::success);
    BOOST_CHECK_EQUAL(model.refresh_model(&status), err::model_update_error);
}

BOOST_AUTO_TEST_CASE(live_model_no_background_refresh) {
    u::configuration config;
    cfg::create_from_json(JSON_CFG, config);

    config.set(r::name::EH_TEST, "true");
    config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");

    r::live_model model = create_mock_live_model(config);

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
  std::vector<buffer_t> recorded_observations;
  auto mock_observation_sender = get_mock_sender(recorded_observations);

  std::vector<buffer_t> recorded_interactions;
  auto mock_interaction_sender = get_mock_sender(recorded_interactions);

  auto mock_decision_sender = get_mock_sender(r::error_code::success);

  auto mock_data_transport = get_mock_data_transport();
  auto mock_model = get_mock_model();

  auto logger_factory = get_mock_sender_factory(mock_observation_sender.get(), mock_interaction_sender.get(), mock_decision_sender.get());
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
    Verify(Method((*mock_decision_sender), init)).Exactly(1);
  }
  //std::string recorded_interactions_all;
  //for (size_t i = 0; i < recorded_interactions.size(); ++i) {
  //  recorded_interactions_all += recorded_interactions[i];
  //  if (i + 1 < recorded_interactions.size()) {
  //    recorded_interactions_all += '\n';
  //  }
  //}

  //std::string recorded_observations_all;
  //for (size_t i = 0; i < recorded_observations.size(); ++i) {
  //  recorded_observations_all += recorded_observations[i];
  //  if (i + 1 < recorded_observations.size()) {
  //    recorded_observations_all += '\n';
  //  }
  //}

  //BOOST_CHECK_EQUAL(recorded_interactions_all, expected_interactions);
  //BOOST_CHECK_EQUAL(recorded_observations_all, expected_observations);
}

BOOST_AUTO_TEST_CASE(populate_response_same_size_test) {
    r::api_status status;
    std::vector<std::vector<size_t>> action_ids = {{0,1,2}, {1,2}, {2}};
    std::vector<std::vector<float>> pdfs = {{0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f}};
    std::vector<const char*> event_ids = {"a", "b", "c"};
    std::string model_id = "id";

    r::decision_response resp;

    BOOST_CHECK_EQUAL(populate_response(action_ids, pdfs, event_ids, std::move(model_id), resp, nullptr, &status), err::success);

    BOOST_CHECK(strcmp(resp.get_model_id(), "id") == 0);
    BOOST_CHECK_EQUAL(resp.size(), 3);

    auto it = resp.begin();
    size_t action_id = 0;
    auto& rank_resp = *it++;
    BOOST_CHECK(strcmp(rank_resp.get_event_id(), "a") == 0);
    BOOST_CHECK_EQUAL(rank_resp.get_chosen_action_id(action_id), err::success);
    BOOST_CHECK_EQUAL(action_id, 0);
    BOOST_CHECK_EQUAL(rank_resp.size(), 3);
    BOOST_CHECK_EQUAL((*rank_resp.begin()).action_id, 0);
    BOOST_CHECK_CLOSE((*rank_resp.begin()).probability, 0.8667f, FLOAT_TOL);

    auto& rank_resp1 = *it++;
    BOOST_CHECK(strcmp(rank_resp1.get_event_id(), "b") == 0);
    BOOST_CHECK_EQUAL(rank_resp1.get_chosen_action_id(action_id), err::success);
    BOOST_CHECK_EQUAL(action_id, 1);
    BOOST_CHECK_EQUAL(rank_resp1.size(), 2);
    BOOST_CHECK_EQUAL((*rank_resp1.begin()).action_id, 1);
    BOOST_CHECK_CLOSE((*rank_resp1.begin()).probability, 0.9f, FLOAT_TOL);

    auto& rank_resp2 = *it++;
    BOOST_CHECK(strcmp(rank_resp2.get_event_id(), "c") == 0);
    BOOST_CHECK_EQUAL(rank_resp2.get_chosen_action_id(action_id), err::success);
    BOOST_CHECK_EQUAL(action_id, 2);
    BOOST_CHECK_EQUAL(rank_resp2.size(), 1);
    BOOST_CHECK_EQUAL((*rank_resp2.begin()).action_id, 2);
    BOOST_CHECK_CLOSE((*rank_resp2.begin()).probability, 1.0f, FLOAT_TOL);
}

BOOST_AUTO_TEST_CASE(populate_response_different_size_test) {
    r::api_status status;
    std::vector<std::vector<size_t>> action_ids = {{0,1,2}, {1,2}};
    std::vector<std::vector<float>> pdfs = {{0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f}};
    std::vector<const char*> event_ids = {"a", "b", "c"};
    std::string model_id = "id";

    r::decision_response resp;

    BOOST_CHECK_EQUAL(populate_response(action_ids, pdfs, event_ids, std::move(model_id), resp, nullptr, &status), err::invalid_argument);

    action_ids = {{0,1}, {1,2}, {1}};
    pdfs = {{0.8667f, 0.0667f, 0.0667f}, {0.9f, 0.1f}, {1.0f}};
    BOOST_CHECK_EQUAL(populate_response(action_ids, pdfs, event_ids, std::move(model_id), resp, nullptr, &status), err::invalid_argument);
}

const auto JSON_CCB_CONTEXT = "{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[ { \"TAction\":{\"a1\":\"f1\"} },{\"TAction\":{\"a2\":\"f2\"}}],\"_slots\":[{\"Slot\":{\"a1\":\"f1\"}},{\"Slot\":{\"a1\":\"f1\"}}]}";

BOOST_AUTO_TEST_CASE(ccb_explore_only_mode) {
  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME,"interaction.txt");
  config.set(r::name::OBSERVATION_FILE_NAME,"observation.txt");

  r::api_status status;
  r::live_model model(config);
  BOOST_CHECK_EQUAL(model.init(&status), err::success);

  r::decision_response response;
  BOOST_CHECK_EQUAL(model.request_decision(JSON_CCB_CONTEXT, response), err::success);

  BOOST_CHECK(strcmp(response.get_model_id(), "N/A") == 0);
  BOOST_CHECK_EQUAL(response.size(), 2);

  auto it = response.begin();
  size_t action_id = 0;
  auto& rank_resp = *it++;
  BOOST_CHECK(strcmp(rank_resp.get_event_id(), "") != 0);
  BOOST_CHECK_EQUAL(rank_resp.get_chosen_action_id(action_id), err::success);
  BOOST_CHECK_EQUAL(action_id, 0);
  BOOST_CHECK_EQUAL(rank_resp.size(), 2);
  BOOST_CHECK_EQUAL((*rank_resp.begin()).action_id, 0);
  BOOST_CHECK_CLOSE((*rank_resp.begin()).probability, 1.f, FLOAT_TOL);

  auto& rank_resp1 = *it++;
  BOOST_CHECK(strcmp(rank_resp1.get_event_id(), "") != 0);
  BOOST_CHECK_EQUAL(rank_resp1.get_chosen_action_id(action_id), err::success);
  BOOST_CHECK_EQUAL(action_id, 1);
  BOOST_CHECK_EQUAL(rank_resp1.size(), 1);
  BOOST_CHECK_EQUAL((*rank_resp1.begin()).action_id, 1);
  BOOST_CHECK_CLOSE((*rank_resp1.begin()).probability, 1.f, FLOAT_TOL);
}
