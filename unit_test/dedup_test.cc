#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "dedup.h"

namespace r = reinforcement_learning;
namespace err = reinforcement_learning::error_code;

BOOST_AUTO_TEST_CASE(dedup_remove_unknown_action)
{
  r::dedup_dict dict;
  BOOST_CHECK_EQUAL(false, dict.remove_action(1));
}

BOOST_AUTO_TEST_CASE(dedup_add_remove_action)
{
  r::dedup_dict dict;
  auto id = dict.add_action("abc", 3);
  auto id2 = dict.add_action("abc", 3);

  BOOST_CHECK_EQUAL(id, id2);

  BOOST_CHECK_EQUAL(true, dict.remove_action(id));
  BOOST_CHECK_EQUAL(true, dict.remove_action(id));
  BOOST_CHECK_EQUAL(false, dict.remove_action(id));
}

BOOST_AUTO_TEST_CASE(dedup_add_empty_action)
{
  r::dedup_dict dict;
  auto id = dict.add_action("", 0);
  BOOST_CHECK_EQUAL(true, dict.remove_action(id));
}

BOOST_AUTO_TEST_CASE(dedup_add_get_action)
{
  r::dedup_dict dict;
  auto id = dict.add_action("{abc}", 5);
  auto str = dict.get_action(id);

  BOOST_CHECK_EQUAL(5, str.size());
  BOOST_CHECK_EQUAL("{abc}", str.to_string());
}

BOOST_AUTO_TEST_CASE(dedup_bad_json)
{
  r::dedup_dict dict;
  const char *payload = "{ invalid }";
  std::string p_out;
  r::dedup_dict::action_list_t a_out;

  BOOST_CHECK_EQUAL(err::json_parse_error, dict.transform_payload(payload, p_out, a_out, nullptr));
}

BOOST_AUTO_TEST_CASE(dedup_simple_json)
{
  r::dedup_dict dict;
  std::string payload = R"(
    {
      "s_": "1",
      "_multi": [
        { "b_": "1" },
        { "b_": "2" }
      ],
      "_slots": [ { "a": 10 }, { "b": 20 } ]
    })";

  std::string p_out;
  r::dedup_dict::action_list_t a_out;

  BOOST_CHECK_EQUAL(err::success, dict.transform_payload(payload.c_str(), p_out, a_out, nullptr));
  BOOST_CHECK_EQUAL(2, a_out.size());
  BOOST_CHECK_EQUAL(989852256, a_out[0]);
  BOOST_CHECK_EQUAL(178626470, a_out[1]);

  std::string transformed_payload = R"(
    {
      "s_": "1",
      "_multi": [
        {"__aid":989852256},
        {"__aid":178626470}
      ],
      "_slots": [ { "a": 10 }, { "b": 20 } ]
    })";
  BOOST_CHECK_EQUAL(transformed_payload, p_out);

  BOOST_CHECK_EQUAL(true, dict.remove_action(989852256));
  BOOST_CHECK_EQUAL(true, dict.remove_action(178626470));
  BOOST_CHECK_EQUAL(false, dict.remove_action(989852256));
  BOOST_CHECK_EQUAL(false, dict.remove_action(178626470));
}
