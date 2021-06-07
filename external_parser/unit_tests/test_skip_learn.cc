#include "example_joiner.h"
#include "parse_example_binary.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

void should_not_add_examples(std::string test_file) {
  std::string infile_name = get_test_files_location() + "skip_learn/" + test_file;
  auto vw = VW::initialize("--cb_explore_adf -d " + infile_name +
                           " --binary_parser --quiet",
                           nullptr, false, nullptr, nullptr);

  v_array<example *> examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(vw));

  while (vw->example_parser->reader(vw, examples) > 0) {
    continue;
  }

  BOOST_CHECK_EQUAL(examples.size(), 1);
  BOOST_CHECK_EQUAL(examples[0]->indices.size(), 0); //newline example

  clear_examples(examples, vw);
  VW::finish(*vw);
}

void should_add_examples(std::string test_file) {
  std::string infile_name = get_test_files_location() + "skip_learn/" + test_file;
  auto vw = VW::initialize("--cb_explore_adf -d " + infile_name +
                           " --binary_parser --quiet",
                           nullptr, false, nullptr, nullptr);

  v_array<example *> examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(vw));

  while (vw->example_parser->reader(vw, examples) > 0) {
    continue;
  }

  BOOST_CHECK_EQUAL(examples.size(), 4);
  BOOST_CHECK_EQUAL(examples[0]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[0]->indices[0], 'G');
  BOOST_CHECK_EQUAL(examples[1]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[1]->indices[0], 'T');
  BOOST_CHECK_EQUAL(examples[2]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[2]->indices[0], 'T');
  BOOST_CHECK_EQUAL(examples[3]->indices.size(), 0); // newline example

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_SUITE(skip_learn_with_regular_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_not_activated_outcome_is_not_learnable) {
  //file contains 1 interaction with deferred action and 1 unactivated observation
  std::string test_file = "cb_deferred_action_without_activation.fb";
  should_not_add_examples(test_file);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable) {
  //file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "cb_deferred_action_with_activation.fb";
  should_add_examples(test_file);
}


BOOST_AUTO_TEST_CASE(only_leanabled_events_should_be_added_to_examples) {
  //file contains 2 interaction, first interaction is deferred action.
  //observations are not activated
  //only second joined event should be added to examples
  std::string test_file = "cb_mixed_deferred_action_events.fb";
  should_add_examples(test_file);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(skip_learn_with_compressed_and_deduped_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_not_activated_outcome_is_not_learnable) {
  //file contains 1 interaction with deferred action and 1 unactivated observation
  std::string test_file = "cb_deferred_action_without_activation_deduped.fb";
  should_not_add_examples(test_file);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable) {
  //file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "cb_deferred_action_with_activation_deduped.fb";
  should_add_examples(test_file);
}
BOOST_AUTO_TEST_SUITE_END()

