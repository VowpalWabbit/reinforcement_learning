#include <boost/test/unit_test.hpp>

#include "common_test_utils.h"
#include "constants.h"
#include "live_model.h"

using namespace reinforcement_learning;

BOOST_AUTO_TEST_CASE(local_loop_end_to_end_test)
{
  utility::configuration config;
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE,
      "--cb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A --preserve_performance_counters");
  config.set(name::PROTOCOL_VERSION, "2");
  config.set(name::EUD_DURATION, "0:0:1");
  config.set(name::MODEL_SRC, value::LOCAL_LOOP_MODEL_DATA);
  config.set(name::INTERACTION_SENDER_IMPLEMENTATION, value::LOCAL_LOOP_SENDER);
  config.set(name::OBSERVATION_SENDER_IMPLEMENTATION, value::LOCAL_LOOP_SENDER);
  config.set(name::JOINER_PROBLEM_TYPE, value::PROBLEM_TYPE_CB);
  config.set(name::JOINER_REWARD_FUNCTION, value::REWARD_FUNCTION_EARLIEST);
  config.set(name::JOINER_LEARNING_MODE, value::LEARNING_MODE_ONLINE);
  config.set(name::MODEL_BACKGROUND_REFRESH, "false");

  api_status status;
  live_model lm(config);
  lm.init(&status);
  BOOST_TEST(status.get_error_code() == error_code::success, status.get_error_msg());
}
