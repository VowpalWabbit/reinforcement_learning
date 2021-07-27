#include "test_common.h"
#include <boost/test/unit_test.hpp>
#include <stdio.h>

std::string get_json_event(std::string infile_path, std::string outfile_path) {
  std::string infile_name = get_test_files_location() + infile_path;
  auto vw = VW::initialize("--cb_explore_adf -d " + infile_name +
                           " --binary_parser --quiet --binary_to_json",
                           nullptr, false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  while (vw->example_parser->reader(vw, examples) > 0) {
    examples.push_back(&VW::get_unused_example(vw));
  }

  std::ostringstream dsjson_stream;
  std::string outfile_name = get_test_files_location() + outfile_path;

  std::ifstream dsjson_output(outfile_name);
  dsjson_stream << dsjson_output.rdbuf();

  clear_examples(examples, vw);
  VW::finish(*vw);
  remove(outfile_name.c_str());

  return dsjson_stream.str();
}

BOOST_AUTO_TEST_CASE(convert_binary_to_dsjson) {
  std::string infile_path = "/valid_joined_logs/cb_simple.log";
  std::string outfile_path = "/valid_joined_logs/cb_simple.dsjson";

  std::string converted_json = get_json_event(infile_path, outfile_path);
  std::string expected_joined_json = "{\"_label_cost\":-1.5,"
    "\"_label_probability\":0.9000000357627869,\"_label_Action\":1,"
    "\"_labelIndex\":0,\"o\":[{\"v\":1.5,\"EventId\":\"91f71c8\","
    "\"ActionTaken\":false}],\"Timestamp\":\"2021-04-13T15:08:46.000000Z\","
    "\"Version\":\"1\",\"EventId\":\"91f71c8\",\"a\":[1,2],"
    "\"c\":{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},"
    "\"_multi\":[{\"TAction\":{\"a1\":\"f1\"}},{\"TAction\":{\"a2\":\"f2\"}}]},"
    "\"p\":[0.9000000357627869,0.10000000149011612],\"VWState\":{\"m\":\"N/A\"},"
    "\"_original_label_cost\":-1.5}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_joined_json);
}

BOOST_AUTO_TEST_CASE(convert_inactive_event_without_activation) {
  std::string infile_path = "/skip_learn/cb/deferred_action_without_activation.fb";
  std::string outfile_path = "/skip_learn/cb/deferred_action_without_activation.dsjson";

  std::string converted_json = get_json_event(infile_path, outfile_path);
  std::string expected_joined_json = "{\"_label_cost\":-1.5,"
    "\"_label_probability\":0.9000000357627869,\"_label_Action\":1,"
    "\"_labelIndex\":0,\"_skipLearn\":true,\"o\":[{\"v\":1.5,"
    "\"EventId\":\"91f71c8\",\"ActionTaken\":false}],"
    "\"Timestamp\":\"2021-05-21T18:54:38.000000Z\",\"Version\":\"1\","
    "\"EventId\":\"91f71c8\",\"a\":[1,2],\"c\":{\"GUser\":{\"id\":\"a\","
    "\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[{\"TAction\":"
    "{\"a1\":\"f1\"}},{\"TAction\":{\"a2\":\"f2\"}}]},\"p\":[0.9000000357627869,"
    "0.10000000149011612],\"VWState\":{\"m\":\"N/A\"},\"_original_label_cost\":-1.5}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_joined_json);
}

BOOST_AUTO_TEST_CASE(convert_inactive_event_with_activation) {
  std::string infile_path = "/skip_learn/cb/deferred_action_with_activation.fb";
  std::string outfile_path = "/skip_learn/cb/deferred_action_with_activation.dsjson";

  std::string converted_json = get_json_event(infile_path, outfile_path);
  std::string expected_joined_json = "{\"_label_cost\":-0.0,"
    "\"_label_probability\":0.9000000357627869,\"_label_Action\":1,"
    "\"_labelIndex\":0,\"o\":[{\"EventId\":\"91f71c8\",\"ActionTaken\":true}],"
    "\"Timestamp\":\"2021-06-04T17:40:13.000000Z\",\"Version\":\"1\","
    "\"EventId\":\"91f71c8\",\"a\":[1,2],\"c\":{\"GUser\":{\"id\":\"a\","
    "\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[{\"TAction\":"
    "{\"a1\":\"f1\"}},{\"TAction\":{\"a2\":\"f2\"}}]},\"p\":[0.9000000357627869,"
    "0.10000000149011612],\"VWState\":{\"m\":\"N/A\"},\"_original_label_cost\":-0.0}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_joined_json);
}