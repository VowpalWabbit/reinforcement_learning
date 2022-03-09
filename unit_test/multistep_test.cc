#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "multistep.h"
#include "ranking_response.h"

using namespace reinforcement_learning;

BOOST_AUTO_TEST_CASE(multistep_depth_is_added)
{
    episode_history history;
    ranking_response resp;
    const std::string context = R"({"f":1})";

    const std::string first = history.get_context(nullptr, context.c_str());
    BOOST_CHECK_EQUAL(R"({"episode":{"depth":"1"},"f":1})", first.c_str());
    history.update("0", nullptr, context.c_str(), resp);
    
    const std::string second = history.get_context("0", context.c_str());
    BOOST_CHECK_EQUAL(R"({"episode":{"depth":"2"},"f":1})", second.c_str());
}
