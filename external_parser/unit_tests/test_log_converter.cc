#include "test_common.h"
#include <boost/test/unit_test.hpp>
#include <stdio.h>

BOOST_AUTO_TEST_CASE(convert_binary_to_dsjson) {
  std::string infile_name = get_test_files_location() + "cb_simple.log";
  auto vw = VW::initialize("--cb_explore_adf -d " + infile_name +
                           " --binary_parser --quiet --binary_to_json",
                           nullptr, false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  while (vw->example_parser->reader(vw, examples) > 0) {
    examples.push_back(&VW::get_unused_example(vw));
  }

  std::ostringstream dsjson_stream;
  std::string outfile_name = get_test_files_location() + "cb_simple.dsjson";
  std::ifstream dsjson_output(outfile_name);
  dsjson_stream << dsjson_output.rdbuf();

  std::string expected_joined_dsjson = "{\"_label_cost\":-1.5,"
    "\"_label_probability\":0.9000000357627869,\"_label_Action\":1,"
    "\"_labelIndex\":0,\"o\":[{\"v\":1.5,\"EventId\":\"91f71c8\","
    "\"ActionTaken\":false}],\"Timestamp\":\"joiner_timestamp\","
    "\"Version\":\"1\",\"EventId\":\"91f71c8\",\"a\":[1,2],"
    "\"c\":{\"GUser\":{\"id\":\"a\",\"major\":\"eng\",\"hobby\":\"hiking\"},"
    "\"_multi\":[{\"TAction\":{\"a1\":\"f1\"}},{\"TAction\":{\"a2\":\"f2\"}}]},"
    "\"p\":[0.9000000357627869,0.10000000149011612],\"VWState\":{\"m\":\"N/A\"},"
    "\"_original_label_cost\":-0.0}\n";

  BOOST_CHECK_EQUAL(dsjson_stream.str(), expected_joined_dsjson);
  clear_examples(examples, vw);
  VW::finish(*vw);
  remove(outfile_name.c_str());
}