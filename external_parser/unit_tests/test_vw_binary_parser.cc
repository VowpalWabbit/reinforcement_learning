#include "joiners/example_joiner.h"
#include "parse_example_binary.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_log_file_with_bad_magic) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/invalid_joined_logs/bad_magic.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  set_buffer_as_vw_input(buffer, vw);
  unsigned int payload_type;
  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                      vw->example_parser->input, payload_type),
                  true);
  BOOST_CHECK_NE(payload_type, MSG_TYPE_FILEMAGIC);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_bad_version) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/invalid_joined_logs/bad_version.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  unsigned int payload_type;

  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                      vw->example_parser->input, payload_type),
                  true);
  BOOST_CHECK_EQUAL(payload_type, MSG_TYPE_FILEMAGIC);

  BOOST_CHECK_EQUAL(bp.read_version(vw->example_parser->input), false);

  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_empty_msg_header) {
  std::string input_files = get_test_files_location();

  auto buffer =
      read_file(input_files + "/invalid_joined_logs/empty_msg_hdr.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  unsigned int payload_type;

  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                      vw->example_parser->input, payload_type),
                  true);
  BOOST_CHECK_EQUAL(payload_type, MSG_TYPE_FILEMAGIC);
  BOOST_CHECK_EQUAL(bp.read_version(vw->example_parser->input), true);

  BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                      vw->example_parser->input, payload_type),
                  true);
  BOOST_CHECK_EQUAL(payload_type, MSG_TYPE_HEADER);
  BOOST_CHECK_EQUAL(bp.read_header(vw->example_parser->input), true);

  BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                        vw->example_parser->input, payload_type),
                    true);
  BOOST_CHECK_EQUAL(payload_type, MSG_TYPE_CHECKPOINT);
  BOOST_CHECK_EQUAL(bp.read_checkpoint_msg(vw->example_parser->input),
                    true);

  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_no_msg_header) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/invalid_joined_logs/no_msg_hdr.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  unsigned int payload_type;

  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                      vw->example_parser->input, payload_type),
                  true);
  BOOST_CHECK_EQUAL(payload_type, MSG_TYPE_FILEMAGIC);
  BOOST_CHECK_EQUAL(bp.read_version(vw->example_parser->input), true);
  BOOST_CHECK_EQUAL(bp.read_header(vw->example_parser->input), false);

  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_unknown_msg_type) {
  std::string input_files = get_test_files_location();

  // test first goes through the file to check it is as expected up until the
  // point where it is supposed to break (since this check is at the very end)
  // so that we know that parse_examples returns false for the correct reason in
  // this test
  {
    auto buffer =
        read_file(input_files + "/invalid_joined_logs/one_invalid_msg_type.log");

    auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet",
                             nullptr, false, nullptr, nullptr);
    set_buffer_as_vw_input(buffer, vw);

    VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
    unsigned int payload_type;

    BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                        vw->example_parser->input, payload_type),
                    true);
    BOOST_CHECK_EQUAL(payload_type, MSG_TYPE_FILEMAGIC);
    BOOST_REQUIRE_EQUAL(bp.read_version(vw->example_parser->input), true);

    BOOST_CHECK_EQUAL(bp.advance_to_next_payload_type(
                        vw->example_parser->input, payload_type),
                    true);
    BOOST_CHECK_EQUAL(payload_type, MSG_TYPE_HEADER);
    BOOST_REQUIRE_EQUAL(bp.read_header(vw->example_parser->input), true);

    BOOST_REQUIRE_EQUAL(bp.advance_to_next_payload_type(
                            vw->example_parser->input, payload_type),
                        true);
    BOOST_REQUIRE_EQUAL(payload_type, MSG_TYPE_CHECKPOINT);
    BOOST_REQUIRE_EQUAL(bp.read_checkpoint_msg(vw->example_parser->input),
                        true);

    // unknown header message
    BOOST_REQUIRE_EQUAL(bp.advance_to_next_payload_type(
                            vw->example_parser->input, payload_type),
                        true);
    BOOST_REQUIRE_NE(payload_type, MSG_TYPE_CHECKPOINT);
    BOOST_REQUIRE_NE(payload_type, MSG_TYPE_EOF);
    BOOST_REQUIRE_NE(payload_type, MSG_TYPE_HEADER);
    BOOST_REQUIRE_NE(payload_type, MSG_TYPE_REGULAR);

    BOOST_REQUIRE_EQUAL(bp.skip_over_unknown_payload(vw->example_parser->input), true);

    // regular message type
    BOOST_REQUIRE_EQUAL(bp.advance_to_next_payload_type(
                            vw->example_parser->input, payload_type),
                        true);
    BOOST_REQUIRE_EQUAL(payload_type, MSG_TYPE_REGULAR);
    v_array<example *> examples;
    examples.push_back(VW::new_unused_example(*vw));
    bool ignore_msg = false;
    BOOST_REQUIRE_EQUAL(bp.read_regular_msg(vw->example_parser->input, examples, ignore_msg),
                        true);
    BOOST_REQUIRE_EQUAL(ignore_msg, false);

    BOOST_REQUIRE_EQUAL(examples.size(), 4);
    clear_examples(examples, vw);
    VW::finish(*vw);
  }
  // now actually check the parser makes it to the end without failing
  {
    auto buffer =
        read_file(input_files + "/invalid_joined_logs/one_invalid_msg_type.log");

    auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet",
                             nullptr, false, nullptr, nullptr);
    set_buffer_as_vw_input(buffer, vw);
    VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
    v_array<example *> examples;
    examples.push_back(VW::new_unused_example(*vw));

    size_t total_examples_read = 0;

    while (bp.parse_examples(vw, vw->example_parser->input, examples)) {
      total_examples_read++;
      clear_examples(examples, vw);
      examples.push_back(VW::new_unused_example(*vw));
    }

    BOOST_CHECK_EQUAL(total_examples_read, 1);
    clear_examples(examples, vw);
    VW::finish(*vw);
  }
}

BOOST_AUTO_TEST_CASE(test_log_file_with_corrupt_joined_event_payload) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files +
                          "/invalid_joined_logs/corrupt_joined_payload.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  v_array<example *> examples;
  examples.push_back(VW::new_unused_example(*vw));

  size_t total_size_of_examples = 0;
  // file contains 2 regular messages, first one contains a corrupted
  // JoinedPayload flatbuffer should be skipped without causing any problems and
  // the second regular message should be consumed
  while (bp.parse_examples(vw, vw->example_parser->input, examples)) {
    BOOST_CHECK_EQUAL(examples.size(), 4);
    total_size_of_examples += examples.size();
    clear_examples(examples, vw);
    examples.push_back(VW::new_unused_example(*vw));
  }

  // skipped first payload and read the second one
  BOOST_CHECK_EQUAL(total_size_of_examples, 4);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_mismatched_payload_types) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(
      input_files + "/invalid_joined_logs/incomplete_checkpoint_info.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  v_array<example *> examples;
  examples.push_back(VW::new_unused_example(*vw));

  size_t total_size_of_examples = 0;
  // file contains 2 regular messages, both have wrong types set
  while (bp.parse_examples(vw, vw->example_parser->input, examples)) {
    total_size_of_examples += examples.size();
  }

  // skipped first payload and read the second one
  BOOST_CHECK_EQUAL(total_size_of_examples, 0);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_bad_event_in_joined_event) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files +
                          "/invalid_joined_logs/bad_event_in_joined_event.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw),vw->logger);
  v_array<example *> examples;
  examples.push_back(VW::new_unused_example(*vw));

  size_t total_size_of_examples = 0;
  // file contains 2 regular messages each with one JoinedEvent that holds one
  // interaction and one observation, first JE has malformed event
  while (bp.parse_examples(vw, vw->example_parser->input, examples)) {
    BOOST_CHECK_EQUAL(examples.size(), 4);
    total_size_of_examples += examples.size();
    clear_examples(examples, vw);
    examples.push_back(VW::new_unused_example(*vw));
  }

  // skipped first payload and read the second one
  BOOST_CHECK_EQUAL(total_size_of_examples, 4);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_dedup_payload_missing) {
  std::string input_files = get_test_files_location();

  auto buffer =
      read_file(input_files + "/invalid_joined_logs/dedup_payload_missing.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  v_array<example *> examples;
  examples.push_back(VW::new_unused_example(*vw));

  size_t total_size_of_examples = 0;
  // file contains 1 regular message with one JoinedEvent that holds one
  // interaction and one observation, but the dedup payload is missing so the
  // payload should not be processed
  while (bp.parse_examples(vw, vw->example_parser->input, examples)) {
    total_size_of_examples += examples.size();
    clear_examples(examples, vw);
    examples.push_back(VW::new_unused_example(*vw));
  }

  // skipped payload
  BOOST_CHECK_EQUAL(total_size_of_examples, 0);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_interaction_but_no_observation) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(
      input_files + "/invalid_joined_logs/interaction_with_no_observation.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  v_array<example *> examples;
  examples.push_back(VW::new_unused_example(*vw));

  size_t total_size_of_examples = 0;
  // file contains 1 regular message with 1 JoinedEvent that holds one
  // interaction with a missing observation, and then another interaction with
  // its observation

  BOOST_CHECK_EQUAL(bp.parse_examples(vw, vw->example_parser->input, examples), true);
  BOOST_CHECK_EQUAL(examples.size(), 4);
  total_size_of_examples += examples.size();
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs.size(), 1);
  // default reward since first observation is missing
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].cost, 0.0f);
  clear_examples(examples, vw);
  examples.push_back(VW::new_unused_example(*vw));

  BOOST_CHECK_EQUAL(bp.parse_examples(vw, vw->example_parser->input, examples), true);
  BOOST_CHECK_EQUAL(examples.size(), 4);
  total_size_of_examples += examples.size();
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs.size(), 1);
  // second interaction should have cost set
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].cost, -1.5f);
  clear_examples(examples, vw);
  examples.push_back(VW::new_unused_example(*vw));

  BOOST_CHECK_EQUAL(bp.parse_examples(vw, vw->example_parser->input, examples), false);

  // both interactions processed
  BOOST_CHECK_EQUAL(total_size_of_examples, 8);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_no_interaction_with_observation) {
  std::string input_files = get_test_files_location();

  auto buffer =
      read_file(input_files +
                "/invalid_joined_logs/no_interaction_but_with_observation.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  v_array<example *> examples;
  examples.push_back(VW::new_unused_example(*vw));

  size_t total_size_of_examples = 0;
  // file contains 1 regular message with 1 JoinedEvent that holds one
  // two observations (i.e. missing interaction), and then another interaction
  // with its observation
  while (bp.parse_examples(vw, vw->example_parser->input, examples)) {
    BOOST_CHECK_EQUAL(examples.size(), 4);
    total_size_of_examples += examples.size();
    clear_examples(examples, vw);
    examples.push_back(VW::new_unused_example(*vw));
  }

  // one interaction processed
  BOOST_CHECK_EQUAL(total_size_of_examples, 4);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_log_file_with_invalid_cb_context) {
  std::string input_files = get_test_files_location();

  auto buffer =
      read_file(input_files + "/invalid_joined_logs/invalid_cb_context.log");
  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);
  set_buffer_as_vw_input(buffer, vw);
  VW::external::binary_parser bp(VW::make_unique<example_joiner>(vw), vw->logger);
  v_array<example *> examples;
  examples.push_back(VW::new_unused_example(*vw));

  size_t total_size_of_examples = 0;
  // file contains 1 regular message with 2 JoinedEvent that has two
  // interactions with their corresponding observations but both cb json
  // context's are wrong (i.e. it is a ccb context and so the json parser will
  // throw) and as a result we won't process anything
  while (bp.parse_examples(vw, vw->example_parser->input, examples)) {
    total_size_of_examples += examples.size();
    clear_examples(examples, vw);
    examples.push_back(VW::new_unused_example(*vw));
  }

  // no interactions processed
  BOOST_CHECK_EQUAL(total_size_of_examples, 0);

  clear_examples(examples, vw);
  VW::finish(*vw);
}
