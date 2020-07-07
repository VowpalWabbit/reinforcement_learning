#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "learning_mode.h"
#include "constants.h"

BOOST_AUTO_TEST_CASE(learning_mode_apprentice_lower_case) {
  const char* apprentice = "apprentice";
  auto mode = reinforcement_learning::learning::to_learning_mode(apprentice);
  BOOST_CHECK_EQUAL(mode, reinforcement_learning::APPRENTICE);
}

BOOST_AUTO_TEST_CASE(learning_mode_apprentice) {
  const char* apprentice = reinforcement_learning::value::LEARNING_MODE_APPRENTICE;
  auto mode = reinforcement_learning::learning::to_learning_mode(apprentice);
  BOOST_CHECK_EQUAL(mode, reinforcement_learning::APPRENTICE);
}

BOOST_AUTO_TEST_CASE(learning_mode_loggingonly_lower_case) {
    const char* logging_only = "loggingonly";
    auto mode = reinforcement_learning::learning::to_learning_mode(logging_only);
    BOOST_CHECK_EQUAL(mode, reinforcement_learning::LOGGINGONLY);
}

BOOST_AUTO_TEST_CASE(learning_mode_loggingonly) {
    const char* logging_only = reinforcement_learning::value::LEARNING_MODE_LOGGINGONLY;
    auto mode = reinforcement_learning::learning::to_learning_mode(logging_only);
    BOOST_CHECK_EQUAL(mode, reinforcement_learning::LOGGINGONLY);
}

BOOST_AUTO_TEST_CASE(learning_mode_online) {
  const char* online = reinforcement_learning::value::LEARNING_MODE_ONLINE;
  auto mode = reinforcement_learning::learning::to_learning_mode(online);
  BOOST_CHECK_EQUAL(mode, reinforcement_learning::ONLINE);
}

BOOST_AUTO_TEST_CASE(learning_mode_other) {
  const char* apprentice = "other";
  auto mode = reinforcement_learning::learning::to_learning_mode(apprentice);
  BOOST_CHECK_EQUAL(mode, reinforcement_learning::ONLINE);
}
