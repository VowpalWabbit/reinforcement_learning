#include "joiners/example_joiner.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>
namespace v2 = reinforcement_learning::messages::flatbuff::v2;

float get_float_reward(std::string int_file_name, std::string obs_file_name, v2::RewardFunctionType type, v2::LearningModeType learning_mode=v2::LearningModeType_Online) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);
  v_array<example *> examples = v_init<example*>();
  example_joiner joiner(vw);
  joiner.set_problem_type_config(v2::ProblemType_CB);
  joiner.set_reward_function(type);

  if (learning_mode != v2::LearningModeType_Online) {
    joiner.set_learning_mode_config(learning_mode);
  }

  std::string input_files = get_test_files_location();
  auto interaction_buffer = read_file(input_files + "/reward_functions/" + int_file_name);
  auto observation_buffer = read_file(input_files + "/reward_functions/" + obs_file_name);

  // need to keep the fb buffer around in order to process the event
  std::vector<flatbuffers::DetachedBuffer> int_detached_buffers;
  auto joined_int_events = wrap_into_joined_events(interaction_buffer, int_detached_buffers);

  for (auto &je : joined_int_events) {
    joiner.process_event(*je);
    examples.push_back(&VW::get_unused_example(vw));
  }

  // need to keep the fb buffer around in order to process the event
  std::vector<flatbuffers::DetachedBuffer> obs_detached_buffers;
  auto joined_obs_events = wrap_into_joined_events(observation_buffer, obs_detached_buffers);

  for (auto &je : joined_obs_events) {
    joiner.process_event(*je);
  }

  joiner.process_joined(examples);

  // works with CB for now
  float reward = 0.0f;
  for (auto *example : examples) {
    if (example->l.cb.costs.size() > 0) {
      // found label
      reward = -1.0 * example->l.cb.costs[0].cost;
    }
  }

  clear_examples(examples, vw);
  VW::finish(*vw);
  return reward;
}

BOOST_AUTO_TEST_SUITE(reward_functions_with_cb_format_and_float_reward)

// 3 rewards in f-reward_3obs_v2.fb are: 5, 4, 3 and timestamps are in descending order added in test_common.cc
BOOST_AUTO_TEST_CASE(earliest) {
  float reward = get_float_reward(
      "cb_v2.fb",
      "f-reward_3obs_v2.fb",
      v2::RewardFunctionType_Earliest
    );

  BOOST_CHECK_EQUAL(reward, 3);
}

BOOST_AUTO_TEST_CASE(average) {
  float reward = get_float_reward(
      "cb_v2.fb",
      "f-reward_3obs_v2.fb",
      v2::RewardFunctionType_Average
    );

  BOOST_CHECK_EQUAL(reward, (3.0 + 4 + 5) / 3);
}

BOOST_AUTO_TEST_CASE(min) {
  float reward = get_float_reward(
      "cb_v2.fb",
      "f-reward_3obs_v2.fb",
      v2::RewardFunctionType_Min
    );

  BOOST_CHECK_EQUAL(reward, 3);
}

BOOST_AUTO_TEST_CASE(max) {
  float reward = get_float_reward(
      "cb_v2.fb",
      "f-reward_3obs_v2.fb",
      v2::RewardFunctionType_Max
    );

  BOOST_CHECK_EQUAL(reward, 5);
}

BOOST_AUTO_TEST_CASE(median) {
  float reward = get_float_reward(
      "cb_v2.fb",
      "f-reward_3obs_v2.fb",
      v2::RewardFunctionType_Median
    );

  BOOST_CHECK_EQUAL(reward, 4);
}

BOOST_AUTO_TEST_CASE(sum) {
  float reward = get_float_reward(
      "cb_v2.fb",
      "f-reward_3obs_v2.fb",
      v2::RewardFunctionType_Sum
    );

  BOOST_CHECK_EQUAL(reward, 3 + 4 + 5);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(reward_functions_with_cb_format_and_appentice_mode)
BOOST_AUTO_TEST_CASE(apprentice_with_first_action_matching_baseline_action_returns_real_reward) {
  // 3 rewards in f-reward_3obs_v2.fb are: 5, 4, 3
  float reward = get_float_reward(
      "cb_apprentice_match_baseline_v2.fb",
      "f-reward_3obs_v2.fb",
      v2::RewardFunctionType_Sum,
      v2::LearningModeType_Apprentice
    );
  BOOST_CHECK_EQUAL(reward, 3 + 4 + 5);
}

// TODO: add test case for first action not matching basesline returns default reward
BOOST_AUTO_TEST_SUITE_END()