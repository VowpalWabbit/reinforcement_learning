#include "test_common.h"
#include <boost/test/unit_test.hpp>
#include <stdio.h>

std::string get_json_event(std::string infile_path, std::string outfile_path,
    v2::ProblemType problem_type=v2::ProblemType_CB) {
  std::string infile_name = get_test_files_location() + infile_path;
  std::string command;

  switch (problem_type) {
    case v2::ProblemType_CB:
      command = "--quiet --binary_to_json --binary_parser --cb_explore_adf -d " + infile_name;
      break;
    case v2::ProblemType_CCB:
      command = "--quiet --binary_to_json --binary_parser --ccb_explore_adf -d " + infile_name;
      break;
    case v2::ProblemType_SLATES:
      command = "--quiet --binary_to_json --binary_parser --ccb_explore_adf --slates -d " + infile_name;
      break;
    case v2::ProblemType_CA:
      command =
          "--quiet --binary_to_json --binary_parser --cats 4 --min_value 1 "
          "--max_value 100 --bandwidth 1 -d" +
          infile_name;
      break;
  }

  auto vw = VW::initialize(command, nullptr, false, nullptr, nullptr);

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

BOOST_AUTO_TEST_SUITE(log_converter_cb_format)
BOOST_AUTO_TEST_CASE(convert_binary_to_dsjson) {
  std::string infile_path = "valid_joined_logs/cb_simple.log";
  std::string outfile_path = "valid_joined_logs/cb_simple.dsjson";

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
  std::string infile_path = "skip_learn/cb/deferred_action_without_activation.fb";
  std::string outfile_path = "skip_learn/cb/deferred_action_without_activation.dsjson";

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
  std::string infile_path = "skip_learn/cb/deferred_action_with_activation.fb";
  std::string outfile_path = "skip_learn/cb/deferred_action_with_activation.dsjson";

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
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(log_converter_ccb_format)
BOOST_AUTO_TEST_CASE(ccb_payload_with_slot_index) {
  std::string infile_path = "valid_joined_logs/ccb_simple.log";
  std::string outfile_path = "valid_joined_logs/ccb_simple.dsjson";

  std::string converted_json = get_json_event(infile_path, outfile_path, v2::ProblemType_CCB);
  std::string expected_joined_json = "{\"Timestamp\":\"2021-06-09T12:08:00.000000Z\","
    "\"Version\":\"1\",\"EventId\":\"91f71c8\",\"c\":{\"GUser\":{\"id\":\"a\","
    "\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[{\"TAction\":{\"a1\":\"f1\"}},"
    "{\"TAction\":{\"a2\":\"f2\"}}],\"_slots\":[{\"Slot\":{\"a1\":\"f1\"}},"
    "{\"Slot\":{\"a1\":\"f1\"}}]},\"_outcomes\":[{\"_label_cost\":-0.0,"
    "\"_id\":\"4a3de951-e9e6-487b-8891-547c9a3fb2480\",\"_a\":[0,1],"
    "\"_p\":[1,0],\"_original_label_cost\":-0.0},"
    "{\"_label_cost\":-1.5,\"_id\":\"4f064c85-a04b-4f3f-bc0e-43aef8ad96530\","
    "\"_a\":[1],\"_p\":[1],\"_o\":[{\"v\":1.5,\"EventId\":\"91f71c8\","
    "\"Index\":\"1\",\"ActionTaken\":false}],\"_original_label_cost\":-1.5}],"
    "\"VWState\":{\"m\":\"N/A\"}}\n";
  BOOST_CHECK_EQUAL(converted_json, expected_joined_json);
}

BOOST_AUTO_TEST_CASE(ccb_payload_with_slot_id) {
  std::string infile_path = "valid_joined_logs/ccb_w_slot_id.log";
  std::string outfile_path = "valid_joined_logs/ccb_w_slot_id.dsjson";

  std::string converted_json = get_json_event(infile_path, outfile_path, v2::ProblemType_CCB);
  std::string expected_json = "{\"Timestamp\":\"2021-08-06T18:29:49.000000Z\","
    "\"Version\":\"1\",\"EventId\":\"91f71c8\",\"c\":{\"GUser\":{\"id\":\"a\","
    "\"major\":\"eng\",\"hobby\":\"hiking\"},\"_multi\":[{\"TAction\":"
    "{\"a1\":\"f1\"}},{\"TAction\":{\"a2\":\"f2\"}}],\"_slots\":[{\"Slot\":"
    "{\"a1\":\"f1\"}, \"_id\": \"slot_0\"},{\"Slot\":{\"a1\":\"f1\"}, \"_id\":"
    "\"slot_1\"}]},\"_outcomes\":[{\"_label_cost\":-1.5,\"_id\":\"slot_0\","
    "\"_a\":[0,1],\"_p\":[1,0],\"_o\":[{\"v\":1.5,\"EventId\":\"91f71c8\","
    "\"Index\":\"slot_0\",\"ActionTaken\":false}],\"_original_label_cost\":-1.5},"
    "{\"_label_cost\":-1.5,\"_id\":\"slot_1\",\"_a\":[1],\"_p\":[1],\"_o\":["
    "{\"v\":1.5,\"EventId\":\"91f71c8\",\"Index\":\"slot_1\",\"ActionTaken\":false}],"
    "\"_original_label_cost\":-1.5}],\"VWState\":{\"m\":\"N/A\"}}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_json);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(log_converter_ca_format)
BOOST_AUTO_TEST_CASE(ca_loop_simple) {
  std::string infile_path = "valid_joined_logs/ca_loop_simple.fb";
  std::string outfile_path = "valid_joined_logs/ca_loop_simple.dsjson";

  std::string converted_json =
      get_json_event(infile_path, outfile_path, v2::ProblemType_CA);
  std::string expected_json =
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.0005050505278632045,"
      "\"action\":1.014871597290039},\"Timestamp\":\"2021-08-24T14:38:15."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"91f71c8\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/A\"}}\n"
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.4755050539970398,"
      "\"action\":12.464624404907227},\"Timestamp\":\"2021-08-24T14:38:15."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"75d50657\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/A\"}}\n"
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.4755050539970398,"
      "\"action\":12.43958568572998},\"Timestamp\":\"2021-08-24T14:38:15."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"e28a9ae6\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/A\"}}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_json);
}

BOOST_AUTO_TEST_CASE(ca_loop_simple_e2e) {
  std::string infile_path = "valid_joined_logs/ca_loop_simple_e2e.log";
  std::string outfile_path = "valid_joined_logs/ca_loop_simple_e2e.dsjson";

  std::string converted_json =
      get_json_event(infile_path, outfile_path, v2::ProblemType_CA);
  std::string expected_json =
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.0005050505278632045,"
      "\"action\":1.014871597290039},\"Timestamp\":\"2021-08-24T16:34:38."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"91f71c8\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/A\"}}\n"
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.4755050539970398,"
      "\"action\":12.464624404907227},\"Timestamp\":\"2021-08-24T16:34:38."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"75d50657\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/A\"}}\n"
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.4755050539970398,"
      "\"action\":12.43958568572998},\"Timestamp\":\"2021-08-24T16:34:38."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"e28a9ae6\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/A\"}}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_json);
}

BOOST_AUTO_TEST_CASE(ca_loop_mixed_skip_learn) {
  std::string infile_path = "valid_joined_logs/ca_loop_mixed_skip_learn.fb";
  std::string outfile_path =
      "valid_joined_logs/ca_loop_mixed_skip_learn.dsjson";

  std::string converted_json =
      get_json_event(infile_path, outfile_path, v2::ProblemType_CA);
  std::string expected_json =
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.0005050505278632045,"
      "\"action\":1.014871597290039},\"Timestamp\":\"2021-08-25T15:36:54."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"91f71c8\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/"
      "A\"},\"_skipLearn\":true}\n"
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.4755050539970398,"
      "\"action\":12.464624404907227},\"Timestamp\":\"2021-08-25T15:36:54."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"75d50657\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/A\"}}\n"
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":0.4755050539970398,"
      "\"action\":12.43958568572998},\"Timestamp\":\"2021-08-25T15:36:54."
      "000000Z\",\"Version\":\"1\",\"EventId\":\"e28a9ae6\",\"c\":{"
      "\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/A\"}}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_json);
}

BOOST_AUTO_TEST_CASE(ca_deferred_action_without_activation) {
  std::string infile_path =
      "skip_learn/ca/deferred_action_without_activation.fb";
  std::string outfile_path =
      "skip_learn/ca/deferred_action_without_activation.dsjson";

  std::string converted_json =
      get_json_event(infile_path, outfile_path, v2::ProblemType_CA);
  std::string expected_json =
      "{\"_label_ca\":{\"cost\":-1.5,\"pdf_value\":"
      "0.0005050505278632045,\"action\":1.014871597290039},\"Timestamp\":"
      "\"2021-08-09T20:57:05.000000Z\",\"Version\":\"1\",\"EventId\":"
      "\"91f71c8\",\"c\":"
      "{\"RobotJoint1\":{\"friction\":78}},\"VWState\":{\"m\":\"N/"
      "A\"},\"_skipLearn\":true}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_json);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(log_converter_slates_format) {
  std::string infile_path = "valid_joined_logs/slates_simple.log";
  std::string outfile_path = "valid_joined_logs/slates_simple.dsjson";

  std::string converted_json = get_json_event(infile_path, outfile_path, v2::ProblemType_SLATES);
  std::string expected_json = "{\"Timestamp\":\"2021-08-20T22:07:29.000000Z\","
  "\"Version\":\"1\",\"EventId\":\"abcdefghijklm\",\"_label_cost\":-1.5,"
  "\"o\":[{\"v\":1.5,\"EventId\":\"abcdefghijklm\",\"ActionTaken\":false}],"
  "\"_outcomes\":[{\"_a\":[0,1],\"_p\":[1,0]},{\"_a\":[0,1,2],\"_p\":[1,0,0]}],"
  "\"c\":{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},"
  "\"_multi\":[{\"TAction\":{\"a1\":\"f1\"},\"_slot_id\":0},{\"TAction\":"
  "{\"a2\":\"f2\"},\"_slot_id\":0},{\"TAction\":{\"a3\":\"f3\"},\"_slot_id\":1},"
  "{\"TAction\":{\"a4\":\"f4\"},\"_slot_id\":1},{\"TAction\":{\"a5\":\"f5\"},"
  "\"_slot_id\":1}],\"_slots\":[{\"Slot\":{\"a1\":\"f1\"}},{\"Slot\":{\"a2\":\"f2\"}}]},"
  "\"VWState\":{\"m\":\"N/A\"}}\n";

  BOOST_CHECK_EQUAL(converted_json, expected_json);
}
