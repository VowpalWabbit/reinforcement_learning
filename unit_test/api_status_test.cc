#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "api_status.h"

int func4(reinforcement_learning::api_status *status, bool use_stream_variant) {
  if (use_stream_variant) {
    RETURN_ERROR_LS(nullptr, status, invalid_argument) << "I failed.";
  } else {
    RETURN_ERROR_ARG(nullptr, status, invalid_argument, "I failed.");
  }
}

int func3(reinforcement_learning::api_status *status, bool use_stream_variant) {
  RETURN_IF_FAIL_WITH_STACK(func4(status, use_stream_variant), status);
  return reinforcement_learning::error_code::success;
}

namespace my_namespace {
int func2_with_ns(reinforcement_learning::api_status *status,
                  bool use_stream_variant) {
  RETURN_IF_FAIL_WITH_STACK(func3(status, use_stream_variant), status);

  return reinforcement_learning::error_code::success;
}
} // namespace my_namespace

int func1(reinforcement_learning::api_status *status, bool use_stream_variant) {
  RETURN_IF_FAIL_WITH_STACK(
      my_namespace::func2_with_ns(status, use_stream_variant), status);

  return reinforcement_learning::error_code::success;
}

BOOST_AUTO_TEST_CASE(api_status_call_stack_test) {
  reinforcement_learning::api_status status;

  // Tests RETURN_ERROR_ARG macro and propagation
  func1(&status, false);
  const auto &stack = status.get_call_stack();
  BOOST_CHECK_EQUAL(stack.size(), 4);
  BOOST_CHECK_EQUAL(stack[0].function_name, "func4");
  BOOST_CHECK_EQUAL(stack[1].function_name, "func3");
  BOOST_CHECK_EQUAL(stack[2].function_name, "func2_with_ns");
  BOOST_CHECK_EQUAL(stack[3].function_name, "func1");

  // Tests RETURN_ERROR_LS macro and propagation
  reinforcement_learning::api_status::try_clear(&status);
  func1(&status, true);
  const auto &stack2 = status.get_call_stack();
  BOOST_CHECK_EQUAL(stack2.size(), 4);
  BOOST_CHECK_EQUAL(stack2[0].function_name, "func4");
  BOOST_CHECK_EQUAL(stack2[1].function_name, "func3");
  BOOST_CHECK_EQUAL(stack2[2].function_name, "func2_with_ns");
  BOOST_CHECK_EQUAL(stack2[3].function_name, "func1");
}
