#include <boost/test/unit_test.hpp>

#include "common_test_utils.h"
#include "constants.h"
#include "federation/local_loop_controller.h"
#include "live_model.h"
#include "ranking_response.h"

#include <chrono>
#include <thread>

using namespace reinforcement_learning;

namespace
{
void pick_context_and_desired_action(std::string& context, size_t& action)
{
  int random = std::rand() % 4;
  switch (random)
  {
    case 0:
      context =
          R"({ "shared":{ "name":"Anna", "time":"Morning" }, "_multi":[{ "TAction":{"article":"Sports"} }, { "TAction":{"article":"Politics"} }, { "TAction":{"article":"Food"} }] })";
      action = 0;
      break;

    case 1:
      context =
          R"({ "shared":{ "name":"Tom", "time":"Morning" }, "_multi":[{ "TAction":{"article":"Sports"} }, { "TAction":{"article":"Politics"} }, { "TAction":{"article":"Food"} }] })";
      action = 1;
      break;

    case 2:
      context =
          R"({ "shared":{ "name":"Anna", "time":"Afternoon" }, "_multi":[{ "TAction":{"article":"Sports"} }, { "TAction":{"article":"Politics"} }, { "TAction":{"article":"Food"} }] })";
      action = 2;
      break;

    case 3:
      context =
          R"({ "shared":{ "name":"Tom", "time":"Afternoon" }, "_multi":[{ "TAction":{"article":"Sports"} }, { "TAction":{"article":"Politics"} }, { "TAction":{"article":"Food"} }] })";
      action = 2;
      break;
  }
}

float run_simulation(live_model& model, api_status& status, int iterations)
{
  float reward = 0.f;
  for (int i = 0; i < iterations; i++)
  {
    std::string context;
    size_t desired_action;
    pick_context_and_desired_action(context, desired_action);

    ranking_response response;
    model.choose_rank(context, response, &status);
    BOOST_TEST(status.get_error_code() == error_code::success, status.get_error_msg());

    size_t chosen_action;
    response.get_chosen_action_id(chosen_action, &status);
    BOOST_TEST(status.get_error_code() == error_code::success, status.get_error_msg());

    float outcome = chosen_action == desired_action ? 1.f : 0.f;
    reward += outcome;
    model.report_outcome(response.get_event_id(), outcome, &status);
    BOOST_TEST(status.get_error_code() == error_code::success, status.get_error_msg());
  }
  return reward;
}

utility::configuration get_test_config()
{
  utility::configuration config;
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE,
      "--cb_explore_adf --json --quiet --epsilon 0.0 --preserve_performance_counters");
  config.set(name::PROTOCOL_VERSION, "2");
  config.set(name::EUD_DURATION, "0:0:1");
  config.set(name::MODEL_SRC, value::LOCAL_LOOP_MODEL_DATA);
  config.set(name::INTERACTION_SENDER_IMPLEMENTATION, value::LOCAL_LOOP_SENDER);
  config.set(name::OBSERVATION_SENDER_IMPLEMENTATION, value::LOCAL_LOOP_SENDER);
  config.set(name::JOINER_PROBLEM_TYPE, value::PROBLEM_TYPE_CB);
  config.set(name::JOINER_REWARD_FUNCTION, value::REWARD_FUNCTION_EARLIEST);
  config.set(name::JOINER_LEARNING_MODE, value::LEARNING_MODE_ONLINE);
  config.set(name::MODEL_BACKGROUND_REFRESH, "false");
  config.set(name::TIME_PROVIDER_IMPLEMENTATION, value::CLOCK_TIME_PROVIDER);
  return config;
}
}  // namespace

BOOST_AUTO_TEST_CASE(local_loop_end_to_end_test)
{
  auto config = get_test_config();
  // config.set(name::TRACE_LOG_IMPLEMENTATION, value::CONSOLE_TRACE_LOGGER);

  // create a custom data_transport_factory_t that saves a pointer
  // to the local_loop_controller that was created
  local_loop_controller* test_local_loop_controller = nullptr;
  data_transport_factory_t test_data_transport_factory;
  test_data_transport_factory.register_type(value::LOCAL_LOOP_MODEL_DATA,
      [&](model_management::i_data_transport** retval, const utility::configuration& cfg, i_trace* trace_logger,
          api_status* status)
      {
        std::unique_ptr<local_loop_controller> output;
        RETURN_IF_FAIL(local_loop_controller::create(output, cfg, trace_logger, status));
        test_local_loop_controller = output.release();
        *retval = static_cast<model_management::i_data_transport*>(test_local_loop_controller);
        return error_code::success;
      });

  // initialize live_model with the custom data transport factory
  api_status status;
  live_model model(
      config, nullptr, nullptr, &reinforcement_learning::trace_logger_factory, &test_data_transport_factory);
  model.init(&status);
  BOOST_TEST(status.get_error_code() == error_code::success, status.get_error_msg());
  BOOST_CHECK_NE(test_local_loop_controller, nullptr);

  // do some inference calls and report the outcome
  constexpr int iterations = 100;
  auto reward_before_update = run_simulation(model, status, iterations);

  // wait for eud and update model
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  model.refresh_model(&status);
  BOOST_TEST(status.get_error_code() == error_code::success, status.get_error_msg());

  // check that updated model has learned from previous outcomes
  model_management::model_data model_data;
  test_local_loop_controller->get_data(model_data, &status);
  BOOST_TEST(status.get_error_code() == error_code::success, status.get_error_msg());
  auto vw = test_utils::create_vw(config.get(name::MODEL_VW_INITIAL_COMMAND_LINE, nullptr), model_data);
  BOOST_CHECK_EQUAL(vw->sd->weighted_labeled_examples, iterations);

  // this may be a bad check due to randomness
  auto reward_after_update = run_simulation(model, status, iterations);
  BOOST_CHECK_GT(reward_after_update, reward_before_update);
}
