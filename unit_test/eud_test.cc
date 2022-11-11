#include <chrono>
#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "federation/eud_utils.h"

#include <functional>
#include <thread>

using namespace reinforcement_learning;

BOOST_AUTO_TEST_CASE(parse_eud_tests)
{
  std::chrono::seconds duration;
  BOOST_CHECK_EQUAL(parse_eud("", duration, nullptr), error_code::invalid_argument);
  BOOST_CHECK_EQUAL(parse_eud("::", duration, nullptr), error_code::invalid_argument);
  BOOST_CHECK_EQUAL(parse_eud("a:b:cc", duration, nullptr), error_code::invalid_argument);
  BOOST_CHECK_EQUAL(parse_eud("1a:1:11", duration, nullptr), error_code::invalid_argument);
  BOOST_CHECK_EQUAL(parse_eud("a1:1:11", duration, nullptr), error_code::invalid_argument);
  BOOST_CHECK_EQUAL(parse_eud("-1:1:11", duration, nullptr), error_code::invalid_argument);

  BOOST_CHECK_EQUAL(parse_eud("1:0:0", duration, nullptr), error_code::success);
  BOOST_CHECK(duration == std::chrono::hours(1));
  BOOST_CHECK_EQUAL(parse_eud("1:25:0", duration, nullptr), error_code::success);
  BOOST_CHECK(duration == std::chrono::hours(1) + std::chrono::minutes(25));
  BOOST_CHECK_EQUAL(parse_eud("1:25:1", duration, nullptr), error_code::success);
  BOOST_CHECK(duration == std::chrono::hours(1) + std::chrono::minutes(25) + std::chrono::seconds(1));
  BOOST_CHECK_EQUAL(parse_eud("83:25:1", duration, nullptr), error_code::success);
  BOOST_CHECK(duration == std::chrono::hours(83) + std::chrono::minutes(25) + std::chrono::seconds(1));
}
