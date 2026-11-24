#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "multistep.h"
#include <boost/test/unit_test.hpp>

#include "err_constants.h"
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

BOOST_AUTO_TEST_CASE(multistep_dump_history_to_json)
{
  {
    // empty history
    episode_state episode("episode0");
    const auto json = episode.dump_history_to_json();
    BOOST_CHECK_EQUAL("{}", json);
  }
  {
    // episode with some events
    episode_state episode("episode0");
    ranking_response resp;
    const std::string context;

    episode.update("event1", nullptr, context, resp);
    episode.update("event2", nullptr, context, resp);
    episode.update("event3", "event1", context, resp);
    episode.update("event4", "event3", context, resp);
    episode.update("event5", "event3", context, resp);
    episode.update("event6", "event5", context, resp);

    const auto json = episode.dump_history_to_json();
    BOOST_CHECK_EQUAL(R"({"event1":1,"event2":1,"event3":2,"event4":3,"event5":3,"event6":4})", json);
  }
}

BOOST_AUTO_TEST_CASE(multistep_init_history_from_json)
{
  {
    // empty history
    episode_state episode("episode0");
    BOOST_CHECK_EQUAL(error_code::success, episode.init_history_from_json("{}"));
    BOOST_CHECK_EQUAL(0, episode.size());
  }
  {
    // episode with some events
    const auto history_json = R"({"event1":1,"event2":1,"event3":2,"event4":3,"event5":3,"event6":4})";
    episode_state episode("episode0");
    BOOST_CHECK_EQUAL(error_code::success, episode.init_history_from_json(history_json));

    const auto& depths = episode.get_history()._test_only_depths();
    BOOST_CHECK_EQUAL(6, depths.size());
    BOOST_CHECK_EQUAL(1, depths.at("event1"));
    BOOST_CHECK_EQUAL(1, depths.at("event2"));
    BOOST_CHECK_EQUAL(2, depths.at("event3"));
    BOOST_CHECK_EQUAL(3, depths.at("event4"));
    BOOST_CHECK_EQUAL(3, depths.at("event5"));
    BOOST_CHECK_EQUAL(4, depths.at("event6"));

    BOOST_CHECK_EQUAL(6, episode.size());
    BOOST_CHECK_EQUAL(history_json, episode.dump_history_to_json());
  }
  {
    // parse error: malformed json
    episode_state episode("episode0");
    BOOST_CHECK_EQUAL(error_code::json_parse_error, episode.init_history_from_json(R"({"111":1)"));
    BOOST_CHECK_EQUAL(0, episode.size());
  }
  {
    // parse error: wrong property type
    episode_state episode("episode0");
    BOOST_CHECK_EQUAL(error_code::json_parse_error, episode.init_history_from_json(R"({111:1})"));
    BOOST_CHECK_EQUAL(0, episode.size());
  }
  {
    // parse error: wrong value type
    episode_state episode("episode0");
    BOOST_CHECK_EQUAL(error_code::json_parse_error, episode.init_history_from_json(R"({"111":"1"})"));
    BOOST_CHECK_EQUAL(0, episode.size());
  }
}
