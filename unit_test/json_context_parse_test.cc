#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <cpprest/json.h>
#include "err_constants.h"
#include "utility/context_helper.h"

#include <map>

using namespace reinforcement_learning;
namespace rlutil = reinforcement_learning::utility;

BOOST_AUTO_TEST_CASE(basic_json_test) {
  auto const context = R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ]
  })";
  size_t count = 0;
  const auto scode = rlutil::get_action_count(count, context, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(count, 2);
}

BOOST_AUTO_TEST_CASE(json_no_multi) {
  auto context = R"({
    "UserAge":15,
    "ulti":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ]
  })";
  size_t count = 0;
  auto scode = rlutil::get_action_count(count, context, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::json_no_actions_found);

  context = R"({"UserAge":15})";
  scode = rlutil::get_action_count(count, context, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::json_no_actions_found);
}

BOOST_AUTO_TEST_CASE(json_malformed) {
  const auto context = R"({"UserAgeq09898u)(**&^(*&^*^* })";
  size_t count = 0;
  const auto scode = rlutil::get_action_count(count, context, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::json_parse_error);
}

BOOST_AUTO_TEST_CASE(event_ids_json_malformed) {
  const auto context = R"({"UserAgeq09898u)(**&^(*&^*^* })";

  std::map<size_t, std::string> found;
  const auto scode = rlutil::get_event_ids(context, found, nullptr, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::json_parse_error);
}

BOOST_AUTO_TEST_CASE(event_ids_json_no_slots) {

  const auto context = R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ],
    "_slots": [
    ]
  })";
  std::map<size_t, std::string> found;
  const auto scode = rlutil::get_event_ids(context, found, nullptr, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(found.size(), 0);
}

BOOST_AUTO_TEST_CASE(event_ids_json_basic) {
  const auto context = R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ],
    "_slots": [
      {"a":4},
      {"_id":"test"}
    ]
  })";
  std::map<size_t, std::string> found;
  const auto scode = rlutil::get_event_ids(context, found, nullptr, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::success);

  BOOST_CHECK_EQUAL(found.size(), 1);
  BOOST_CHECK_EQUAL(found.count(0), 0);
  BOOST_CHECK_EQUAL(found.count(1), 1);
  BOOST_CHECK_EQUAL(found[1], "test");
}


BOOST_AUTO_TEST_CASE(slot_count) {
  const auto context = R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ],
    "_slots": [
      {"a":4},
      {"_id":"test"}
    ]
  })";
  size_t count = 0;
  const auto scode = rlutil::get_slot_count(count, context, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(count, 2);
}


BOOST_AUTO_TEST_CASE(slot_count_empty) {
  const auto context = R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ],
    "_slots": []
  })";
  size_t count = 0;
  const auto scode = rlutil::get_slot_count(count, context, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::json_no_slots_found);
  BOOST_CHECK_EQUAL(count, 0);
}

BOOST_AUTO_TEST_CASE(slots_before_multi) {
  const auto context = R"({
    "UserAge":15,
    "_slots": []
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ],
  })";
  const auto scode = rlutil::validate_multi_before_slots(context, nullptr);
  BOOST_CHECK_EQUAL(scode, error_code::json_parse_error);
}

std::string get_action_str(const std::string &context, rlutil::ContextInfo& info, int idx)
{
  return context.substr(info.actions[idx].first, info.actions[idx].second - info.actions[idx].first + 1);
}

std::string get_slot_str(const std::string &context, rlutil::ContextInfo& info, int idx)
{
  return context.substr(info.slots[idx].first, info.slots[idx].second - info.slots[idx].first + 1);
}

BOOST_AUTO_TEST_CASE(get_context_info_test) {
  const auto context = std::string(R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"},
      { }
    ],
    "_slots": [
      {"a":4},
      {"_id":"test"},
      {},
      {}
    ]
  })");

  rlutil::ContextInfo info;
  const auto scode = rlutil::get_context_info(context.c_str(), info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(3, info.actions.size());
  BOOST_CHECK_EQUAL(4, info.slots.size());
  BOOST_CHECK_EQUAL('{', context[info.actions[0].first]);
  BOOST_CHECK_EQUAL('}', context[info.actions[0].second]);

  BOOST_CHECK_EQUAL("{\"_text\":\"elections maine\", \"Source\":\"TV\"}", get_action_str(context, info, 0));
  BOOST_CHECK_EQUAL("{\"Source\":\"www\", \"topic\":4, \"_label\":\"2:3:.3\"}", get_action_str(context, info, 1));
  BOOST_CHECK_EQUAL("{ }", get_action_str(context, info, 2));

  BOOST_CHECK_EQUAL("{\"a\":4}", get_slot_str(context, info, 0));
  BOOST_CHECK_EQUAL("{}", get_slot_str(context, info, 2));
}