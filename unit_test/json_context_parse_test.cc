#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
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
  rlutil::ContextInfo info;
  const auto scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(info.actions.size(), 2);
}

BOOST_AUTO_TEST_CASE(json_no_multi) {
  auto context = R"({
    "UserAge":15,
    "ulti":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ]
  })";
  rlutil::ContextInfo info;
  auto scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(0, info.actions.size());

  context = R"({"UserAge":15})";
  scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(0, info.actions.size());
}

BOOST_AUTO_TEST_CASE(json_malformed) {
  const auto context = R"({"UserAgeq09898u)(**&^(*&^*^* })";

  rlutil::ContextInfo info;
  auto scode = rlutil::get_context_info(context, info);
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
      {"Source":"www", "topic":4, "_label":"2:3:.3"},
      {}
    ],
    "_slots": [
      {"a":4},
      {"_id":"test"}
    ]
  })";
  rlutil::ContextInfo info;
  auto scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(info.slots.size(), 2);
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
  rlutil::ContextInfo info;
  auto scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(info.slots.size(), 0);
}

BOOST_AUTO_TEST_CASE(slots_before_multi) {
  const auto context = R"({
    "UserAge":15,
    "_slots": [{}],
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ]
  })";
  rlutil::ContextInfo info;
  auto scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  BOOST_CHECK_EQUAL(true, info.slots[0].first < info.actions[0].first);
}

std::string get_action_str(const std::string &context, rlutil::ContextInfo& info, int idx)
{
  return context.substr(info.actions[idx].first, info.actions[idx].second);
}

std::string get_slot_str(const std::string &context, rlutil::ContextInfo& info, int idx)
{
  return context.substr(info.slots[idx].first, info.slots[idx].second);
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
  BOOST_CHECK_EQUAL('}', context[info.actions[0].first + info.actions[0].second - 1]);

  BOOST_CHECK_EQUAL("{\"_text\":\"elections maine\", \"Source\":\"TV\"}", get_action_str(context, info, 0));
  BOOST_CHECK_EQUAL("{\"Source\":\"www\", \"topic\":4, \"_label\":\"2:3:.3\"}", get_action_str(context, info, 1));
  BOOST_CHECK_EQUAL("{ }", get_action_str(context, info, 2));

  BOOST_CHECK_EQUAL("{\"a\":4}", get_slot_str(context, info, 0));
  BOOST_CHECK_EQUAL("{}", get_slot_str(context, info, 2));
}

BOOST_AUTO_TEST_CASE(get_slot_ids_test)
{
  auto const context = R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ],
    "_slots": [
      {"id":"provided_slot_id_1", "a":4},
      {"id":"provided_slot_id_2", "_id":"test"}
    ]
  })";
  rlutil::ContextInfo info;
  auto scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  std::vector<std::string> slot_ids;

  scode = rlutil::get_slot_ids_or_add_string_index(context, info.slots, slot_ids);
  BOOST_CHECK_EQUAL(scode, error_code::success);

  BOOST_CHECK_EQUAL(slot_ids.size(), 2);
  BOOST_CHECK_EQUAL(slot_ids[0], "provided_slot_id_1");
  BOOST_CHECK_EQUAL(slot_ids[1], "provided_slot_id_2");
}


BOOST_AUTO_TEST_CASE(get_slot_ids_no_slot_ids_test)
{
  auto const context = R"({
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
  rlutil::ContextInfo info;
  auto scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  std::vector<std::string> slot_ids;

  scode = rlutil::get_slot_ids_or_add_string_index(context, info.slots, slot_ids);
  BOOST_CHECK_EQUAL(scode, error_code::success);

  BOOST_CHECK_EQUAL(slot_ids.size(), 2);
  BOOST_CHECK_EQUAL(slot_ids[0], "0");
  BOOST_CHECK_EQUAL(slot_ids[1], "1");
}

BOOST_AUTO_TEST_CASE(get_slot_ids_some_slots_missing)
{
  auto const context = R"({
    "UserAge":15,
    "_multi":[
      {"_text":"elections maine", "Source":"TV"},
      {"Source":"www", "topic":4, "_label":"2:3:.3"}
    ],
    "_slots": [
      {"a":4, "id": "provided_id_0"},
      {"_id":"test"},
      {"_id":"test", "id": "provided_id_2"}
    ]
  })";
  rlutil::ContextInfo info;
  auto scode = rlutil::get_context_info(context, info);
  BOOST_CHECK_EQUAL(scode, error_code::success);
  std::vector<std::string> slot_ids;

  scode = rlutil::get_slot_ids_or_add_string_index(context, info.slots, slot_ids);
  BOOST_CHECK_EQUAL(scode, error_code::success);

  BOOST_CHECK_EQUAL(slot_ids.size(), 3);
  BOOST_CHECK_EQUAL(slot_ids[0], "provided_id_0");
  BOOST_CHECK_EQUAL(slot_ids[1], "1");
  BOOST_CHECK_EQUAL(slot_ids[2], "provided_id_2");
}