#include "joiners/example_joiner.h"
#include "parse_example_binary.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

void should_not_add_examples(std::string &infile,
    v2::ProblemType problem_type=v2::ProblemType_CB) {
  std::string command;
  switch (problem_type) {
    case v2::ProblemType_CB:
      command = "--quiet --binary_parser --cb_explore_adf";
      break;
    case v2::ProblemType_CCB:
      command = "--quiet --binary_parser --ccb_explore_adf";
      break;
  }

  std::string infile_name = get_test_files_location() + "skip_learn/" + infile;
  auto vw = VW::initialize(command + " -d " + infile_name,
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

void should_add_examples(std::string &infile,
    v2::ProblemType problem_type=v2::ProblemType_CB) {
  std::string command;
  switch (problem_type) {
    case v2::ProblemType_CB:
      command = "--quiet --binary_parser --cb_explore_adf";
      break;
    case v2::ProblemType_CCB:
      command = "--quiet --binary_parser --ccb_explore_adf";
      break;
  }

  std::string infile_name = get_test_files_location() + "skip_learn/" + infile;
  auto vw = VW::initialize(command + " -d " + infile_name,
                           nullptr, false, nullptr, nullptr);

  v_array<example *> examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(vw));

  while (vw->example_parser->reader(vw, examples) > 0) {
    continue;
  }

  switch (problem_type) {
    case v2::ProblemType_CB:
      BOOST_CHECK_EQUAL(examples.size(), 4);
      BOOST_CHECK_EQUAL(examples[0]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[0]->indices[0], 'G');
      BOOST_CHECK_EQUAL(examples[1]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[1]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[2]->indices.size(), 1);
      BOOST_CHECK_EQUAL(examples[2]->indices[0], 'T');
      BOOST_CHECK_EQUAL(examples[3]->indices.size(), 0); // newline example
      break;
    case v2::ProblemType_CCB:
      break;
  }

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_SUITE(skip_learn_with_regular_cb_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_not_activated_outcome_is_not_learnable) {
  //file contains 1 interaction with deferred action and 1 unactivated observation
  std::string test_file = "cb/deferred_action_without_activation.fb";
  should_not_add_examples(test_file);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable) {
  //file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "cb/deferred_action_with_activation.fb";
  should_add_examples(test_file);
}


BOOST_AUTO_TEST_CASE(only_leanabled_events_should_be_added_to_examples) {
  //file contains 2 interaction, first interaction is deferred action.
  //observations are not activated
  //only second joined event should be added to examples
  std::string test_file = "cb/mixed_deferred_action_events.fb";
  should_add_examples(test_file);
}
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(skip_learn_with_compressed_and_deduped_cb_payload)
BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_not_activated_outcome_is_not_learnable) {
  //file contains 1 interaction with deferred action and 1 unactivated observation
  std::string test_file = "cb/deferred_action_without_activation_deduped.fb";
  should_not_add_examples(test_file);
}

BOOST_AUTO_TEST_CASE(joined_event_with_deferred_action_and_at_least_one_activated_outcome_is_learnable) {
  //file contains 1 interaction with deferred action and 1 activated observation
  std::string test_file = "cb/deferred_action_with_activation_deduped.fb";
  should_add_examples(test_file);
}
BOOST_AUTO_TEST_SUITE_END()

