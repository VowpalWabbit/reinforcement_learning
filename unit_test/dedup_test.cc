#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "dedup_internals.h"

namespace r = reinforcement_learning;
namespace err = reinforcement_learning::error_code;
namespace fb = flatbuffers;

fb::DetachedBuffer str_to_buff(const char *str)
{
  auto len = strlen(str) + 1;
  uint8_t *copy = fb::DefaultAllocator().allocate(len);
  memcpy(copy, str, len);
  return fb::DetachedBuffer(nullptr, false, copy, 0, copy, len);
}


BOOST_AUTO_TEST_CASE(dedup_remove_unknown_object)
{
  r::dedup_dict dict;
  BOOST_CHECK_EQUAL(false, dict.remove_object(1));
}

BOOST_AUTO_TEST_CASE(dedup_add_remove_object)
{
  r::dedup_dict dict;
  auto id = dict.add_object("abc", 3);
  auto id2 = dict.add_object("abc", 3);

  BOOST_CHECK_EQUAL(id, id2);

  BOOST_CHECK_EQUAL(true, dict.remove_object(id));
  BOOST_CHECK_EQUAL(true, dict.remove_object(id));
  BOOST_CHECK_EQUAL(false, dict.remove_object(id));
}


BOOST_AUTO_TEST_CASE(dedup_remove_object_multiple)
{
  r::dedup_dict dict;
  auto id = dict.add_object("abc", 3);
  dict.add_object("abc", 3);
  dict.add_object("abc", 3);

  BOOST_CHECK_EQUAL(true, dict.remove_object(id, 3));
  BOOST_CHECK_EQUAL(false, dict.remove_object(id));

  //test that passing a count bigger than the one in the dict works
  dict.add_object("abc", 3);
  BOOST_CHECK_EQUAL(true, dict.remove_object(id, 3));
  BOOST_CHECK_EQUAL(false, dict.remove_object(id));

}

BOOST_AUTO_TEST_CASE(dedup_add_empty_object)
{
  r::dedup_dict dict;
  auto id = dict.add_object("", 0);
  BOOST_CHECK_EQUAL(true, dict.remove_object(id));
}

BOOST_AUTO_TEST_CASE(dedup_add_get_object)
{
  r::dedup_dict dict;
  auto id = dict.add_object("{abc}", 5);
  auto str = dict.get_object(id);

  BOOST_CHECK_EQUAL(5, str.size());
  BOOST_CHECK_EQUAL("{abc}", str.to_string());
}

BOOST_AUTO_TEST_CASE(dedup_bad_json)
{
  r::dedup_dict dict;
  const char *payload = "{ invalid }";
  std::string p_out;
  r::generic_event::object_list_t a_out;

  BOOST_CHECK_EQUAL(err::json_parse_error, dict.transform_payload_and_add_objects(payload, p_out, a_out, nullptr));
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
  r::generic_event::object_list_t a_out;

  BOOST_CHECK_EQUAL(err::success, dict.transform_payload_and_add_objects(payload.c_str(), p_out, a_out, nullptr));
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

  BOOST_CHECK_EQUAL(true, dict.remove_object(989852256));
  BOOST_CHECK_EQUAL(true, dict.remove_object(178626470));
  BOOST_CHECK_EQUAL(false, dict.remove_object(989852256));
  BOOST_CHECK_EQUAL(false, dict.remove_object(178626470));
}

BOOST_AUTO_TEST_CASE(compression_transformer)
{
  r::zstd_compressor compressor(1);

  const char *input = "fheu83bf vcnCD,mkfne9";
  auto in = str_to_buff(input);
  auto ptr1 = in.data();

  // Transform modify the input buffer in place
  BOOST_CHECK_EQUAL(err::success, compressor.compress(in, nullptr));
  BOOST_CHECK_NE(ptr1, in.data());
  auto ptr2 = in.data();

  BOOST_CHECK_EQUAL(err::success, compressor.decompress(in, nullptr));
  BOOST_CHECK_NE(ptr2, in.data());
  BOOST_CHECK_EQUAL(input, (char*)in.data());
}

BOOST_AUTO_TEST_CASE(action_dict_builder)
{
  r::utility::configuration c;
  r::dedup_state state(c, nullptr);
  r::action_dict_builder builder(state);

  BOOST_CHECK_EQUAL(0, builder.size());

  //lets add a couple of actions to the dict
  auto id1 = state.get_dict().add_object("abc", 3);
  auto id2 = state.get_dict().add_object("xyz", 3);

  //include them in the dict-builder
  r::generic_event::object_list_t lst = { id1, id2 };

  builder.add(lst);
  BOOST_CHECK_GT(builder.size(), 0);

  r::generic_event evt;
  BOOST_CHECK_EQUAL(r::error_code::success, builder.finalize(evt, nullptr));

  BOOST_CHECK_EQUAL(r::DEDUP_DICT_EVENT_ID, evt.get_id());
  BOOST_CHECK_EQUAL(1, evt.get_pass_prob());
  BOOST_CHECK_EQUAL(r::generic_event::payload_type_t::PayloadType_DedupInfo, evt.get_payload_type());
}

BOOST_AUTO_TEST_CASE(ewma_test)
{
  r::ewma e(1, .5f);
  BOOST_CHECK_EQUAL(1, e.value());
  e.update(1);
  BOOST_CHECK_EQUAL(1, e.value());
  e.update(0);
  BOOST_CHECK_EQUAL(0.5, e.value());
}

BOOST_AUTO_TEST_CASE(dedup_state_expected_use)
{
  //This test the expected usage of it
  r::utility::configuration c;
  r::dedup_state state(c, nullptr);

  BOOST_CHECK_EQUAL(1, state.get_ewma_value());
  state.update_ewma(2);
  BOOST_CHECK_EQUAL(1.5, state.get_ewma_value());

  std::string payload = R"(
    {
      "s_": "1",
      "_multi": [
        { "b_": "1" },
        { "b_": "2" }
      ],
      "_slots": [ { "a": 10 }, { "b": 20 } ]
    })";

  std::string edited_payload;
  r::generic_event::object_list_t obj_list;
  //1st - the payload is edited to extract actions
  BOOST_CHECK_EQUAL(err::success, state.transform_payload_and_add_objects(payload.c_str(), edited_payload, obj_list, nullptr));

  BOOST_CHECK_EQUAL(2, obj_list.size());
  BOOST_CHECK_GT(state.get_dict().get_object(obj_list[0]).size(), 0);
  BOOST_CHECK_GT(state.get_dict().get_object(obj_list[1]).size(), 0);

  //2nd - we get the content for a bunch of payloads
  std::unordered_map<r::generic_event::object_id_t, size_t> batch_actions = {
    { obj_list[0], 1 },
    { obj_list[1], 1 },
  };

  r::generic_event::object_list_t action_ids;
  std::vector<r::string_view> action_values;
  BOOST_CHECK_EQUAL(err::success, state.get_all_values(batch_actions.begin(), batch_actions.end(), action_ids, action_values, nullptr));

  BOOST_CHECK_EQUAL(2, action_ids.size());
  BOOST_CHECK_EQUAL(2, action_values.size());

  //3nd - we remove those ids from dedup_state_dict
  BOOST_CHECK_EQUAL(err::success, state.remove_all_values(batch_actions.begin(), batch_actions.end(), nullptr));
  BOOST_CHECK_EQUAL(state.get_dict().get_object(obj_list[0]).size(), 0);
  BOOST_CHECK_EQUAL(state.get_dict().get_object(obj_list[1]).size(), 0);
}


BOOST_AUTO_TEST_CASE(dedup_state_and_bad_state)
{
  r::utility::configuration c;
  r::dedup_state state(c, nullptr);

  BOOST_CHECK_EQUAL(0, state.get_object(10).size());

  std::unordered_map<r::generic_event::object_id_t, size_t> batch_actions = {
    { 99, 1 },
    { 100, 1 },
  };

  r::generic_event::object_list_t action_ids;
  std::vector<r::string_view> action_values;
  BOOST_CHECK_EQUAL(err::compression_error, state.get_all_values(batch_actions.begin(), batch_actions.end(), action_ids, action_values, nullptr));
  BOOST_CHECK_EQUAL(err::compression_error, state.remove_all_values(batch_actions.begin(), batch_actions.end(), nullptr));

  auto id = state.get_dict().add_object("abc", 3);

  batch_actions = {
    { id, 100 },
  };
  //dedup_dict clamps the remove count to its current count
  BOOST_CHECK_EQUAL(err::success, state.remove_all_values(batch_actions.begin(), batch_actions.end(), nullptr));
  BOOST_CHECK_EQUAL(0, state.get_dict().get_object(id).size());
}

