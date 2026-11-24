#include <boost/test/unit_test.hpp>

#include "rl_string_view.h"

#include <string>

bool is_invoked_with(const std::string& arg)
{
  for (size_t i = 0; i < boost::unit_test::framework::master_test_suite().argc; i++)
  {
    if (reinforcement_learning::string_view(boost::unit_test::framework::master_test_suite().argv[i]).find(arg) !=
        std::string::npos)
    {
      return true;
    }
  }
  return false;
}