#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "learning_mode.h"
#include "constant.h"

BOOST_AUTO_TEST_CASE(learning_mode_lower_case) {
  const char* imitation = "imitation";
  auto mode = reinforcement_learning::learning::to_learning_mode(imitation);
  BOOST_CHECK_EQUAL(mode, reinforcement_learning::IMITATION);
}

BOOST_AUTO_TEST_CASE(learning_mode_imitation) {
  const char* imitation = "IMITATION";
  auto mode = reinforcement_learning::learning::to_learning_mode(imitation);
  BOOST_CHECK_EQUAL(mode, reinforcement_learning::IMITATION);
}

BOOST_AUTO_TEST_CASE(learning_mode_online) {
  const char* imitation = "ONLINE";
  auto mode = reinforcement_learning::learning::to_learning_mode(imitation);
  BOOST_CHECK_EQUAL(mode, reinforcement_learning::ONLINE);
}

BOOST_AUTO_TEST_CASE(learning_mode_other) {
  const char* imitation = "other";
  auto mode = reinforcement_learning::learning::to_learning_mode(imitation);
  BOOST_CHECK_EQUAL(mode, reinforcement_learning::ONLINE);
}
