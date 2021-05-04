#include "parse_example_binary.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_log_file_with_bad_magic) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/bad_magic.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  set_buffer_as_vw_input(buffer, vw);

  VW::external::binary_parser bp(vw);
  BOOST_CHECK_EQUAL(bp.read_magic(vw->example_parser->input.get()), false);

  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_bad_version) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/bad_version.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);

  VW::external::binary_parser bp(vw);
  BOOST_CHECK_EQUAL(bp.read_magic(vw->example_parser->input.get()), true);
  BOOST_CHECK_EQUAL(bp.read_version(vw->example_parser->input.get()), false);

  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_empty_msg_header) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/empty_msg_hdr.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);

  VW::external::binary_parser bp(vw);
  BOOST_CHECK_EQUAL(bp.read_magic(vw->example_parser->input.get()), true);
  BOOST_CHECK_EQUAL(bp.read_version(vw->example_parser->input.get()), true);
  BOOST_CHECK_EQUAL(bp.read_header(vw->example_parser->input.get()), true);
  unsigned int payload_type;
  BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                        vw->example_parser->input.get(), payload_type),
                    true);
  BOOST_CHECK_EQUAL(bp.read_checkpoint_msg(vw->example_parser->input.get()),
                    true);

  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_no_msg_header) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/no_msg_hdr.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);

  VW::external::binary_parser bp(vw);
  BOOST_CHECK_EQUAL(bp.read_magic(vw->example_parser->input.get()), true);
  BOOST_CHECK_EQUAL(bp.read_version(vw->example_parser->input.get()), true);
  BOOST_CHECK_EQUAL(bp.read_header(vw->example_parser->input.get()), false);

  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_unknown_msg_type) {
  std::string input_files = get_test_files_location();

  // test first goes through the file to check it is as expected up until the
  // point where it is supposed to break (since this check is at the very end)
  // so that we know that parse_examples returns false for the correct reason in
  // this test
  {
    auto buffer = read_file(input_files + "/unknown_msg_type.log");

    auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet",
                             nullptr, false, nullptr, nullptr);
    set_buffer_as_vw_input(buffer, vw);

    VW::external::binary_parser bp(vw);
    v_array<example *> examples;

    BOOST_REQUIRE_EQUAL(bp.read_magic(vw->example_parser->input.get()), true);
    BOOST_REQUIRE_EQUAL(bp.read_version(vw->example_parser->input.get()), true);
    BOOST_REQUIRE_EQUAL(bp.read_header(vw->example_parser->input.get()), true);
    unsigned int payload_type;
    BOOST_REQUIRE_EQUAL(bp.advance_to_next_payload_type(
                            vw->example_parser->input.get(), payload_type),
                        true);
    BOOST_REQUIRE_EQUAL(payload_type, MSG_TYPE_CHECKPOINT);
    BOOST_REQUIRE_EQUAL(bp.read_checkpoint_msg(vw->example_parser->input.get()),
                        true);
    BOOST_REQUIRE_EQUAL(bp.advance_to_next_payload_type(
                            vw->example_parser->input.get(), payload_type),
                        true);
    BOOST_REQUIRE_NE(payload_type, MSG_TYPE_CHECKPOINT);
    BOOST_REQUIRE_NE(payload_type, MSG_TYPE_EOF);
    BOOST_REQUIRE_NE(payload_type, MSG_TYPE_HEADER);
    BOOST_REQUIRE_NE(payload_type, MSG_TYPE_REGULAR);
    VW::finish(*vw);
  }
  // now actually check the parser returns false because of unknown MSG_TYPE
  {
    auto buffer = read_file(input_files + "/unknown_msg_type.log");

    auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet",
                             nullptr, false, nullptr, nullptr);
    set_buffer_as_vw_input(buffer, vw);
    VW::external::binary_parser bp(vw);
    v_array<example *> examples;
    BOOST_CHECK_EQUAL(bp.parse_examples(vw, examples), false);
    VW::finish(*vw);
  }
}

BOOST_AUTO_TEST_CASE(test_log_file_with_bad_joined_event_payload) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/bad_joined_payload.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(vw);
  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  size_t total_size_of_examples = 0;
  // file contains 2 regular messages, first one contains a corrupted
  // JoinedPayload flatbuffer should be skipped without causing any problems and
  // the second regular message should be consumed
  while (bp.parse_examples(vw, examples)) {
    BOOST_CHECK_EQUAL(examples.size(), 4);
    total_size_of_examples += examples.size();
    clear_examples(examples, vw);
    examples.push_back(&VW::get_unused_example(vw));
  }

  // skipped first payload and read the second one
  BOOST_CHECK_EQUAL(total_size_of_examples, 4);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_mismatched_payload_types) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(
      input_files + "/incomplete_checkpoint_info.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(vw);
  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  size_t total_size_of_examples = 0;
  // file contains 2 regular messages, both have wrong types set
  while (bp.parse_examples(vw, examples)) {
    total_size_of_examples += examples.size();
  }

  // skipped first payload and read the second one
  BOOST_CHECK_EQUAL(total_size_of_examples, 0);

  clear_examples(examples, vw);
  VW::finish(*vw);
}