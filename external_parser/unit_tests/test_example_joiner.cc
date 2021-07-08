#include "joiners/example_joiner.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(example_joiner_test_cb) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);
  v_array<example *> examples;

  joiner.set_problem_type_config(v2::ProblemType_CB);

  std::string input_files = get_test_files_location();
  auto interaction_buffer = read_file(input_files + "/fb_events/cb_v2.fb");
  // need to keep the fb buffer around in order to process the event
  std::vector<flatbuffers::DetachedBuffer> int_detached_buffers;

  auto joined_cb_events =
      wrap_into_joined_events(interaction_buffer, int_detached_buffers);

  for (auto &je : joined_cb_events) {
    joiner.process_event(*je);
    examples.push_back(&VW::get_unused_example(vw));
  }

  // need to keep the fb buffer around in order to process the event
  std::vector<flatbuffers::DetachedBuffer> obs_detached_buffers;

  auto observation_buffer =
      read_file(input_files + "/fb_events/f-reward_v2.fb");
  joined_cb_events =
      wrap_into_joined_events(observation_buffer, obs_detached_buffers);

  for (auto &je : joined_cb_events) {
    joiner.process_event(*je);
  }

  BOOST_CHECK_EQUAL(joiner.processing_batch(), true);

  joiner.process_joined(examples);

  BOOST_CHECK_EQUAL(examples.size(), 4);
  // first example is shared example
  BOOST_CHECK_EQUAL(CB::ec_is_example_header(*examples[0]), true);
  // second example has label set and non of the other do
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs.size(), 1);
  BOOST_CHECK_EQUAL(examples[2]->l.cb.costs.size(), 0);
  // last example is empty
  BOOST_CHECK_EQUAL(example_is_newline(*examples[3]), true);

  // check label
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].action, 1);
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].cost, -1.5f);
  BOOST_CHECK_CLOSE(examples[1]->l.cb.costs[0].probability, 0.9, FLOAT_TOL);
  BOOST_CHECK_EQUAL(examples[1]->l.cb.weight, 1.0f);

  // check example features
  // from example_gen payload is JSON_CB_CONTEXT =
  // R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}]})";
  BOOST_CHECK_EQUAL(examples[0]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[0]->indices[0], 'G');
  BOOST_CHECK_EQUAL(examples[1]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[1]->indices[0], 'T');
  BOOST_CHECK_EQUAL(examples[2]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[2]->indices[0], 'T');
  BOOST_CHECK_EQUAL(examples[3]->indices.size(), 0); // newline example

  BOOST_CHECK_EQUAL(joiner.processing_batch(), false);
  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(example_joiner_test_cbb) {
  auto vw = VW::initialize("--quiet --binary_parser --ccb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);
  v_array<example *> examples;

  joiner.set_problem_type_config(v2::ProblemType_CCB);

  std::string input_files = get_test_files_location();
  auto interaction_buffer = read_file(input_files + "/fb_events/ccb_v2.fb");
  // need to keep the fb buffer around in order to process the event
  std::vector<flatbuffers::DetachedBuffer> int_detached_buffers;

  auto joined_ccb_events =
      wrap_into_joined_events(interaction_buffer, int_detached_buffers);

  for (auto &je : joined_ccb_events) {
    joiner.process_event(*je);
    examples.push_back(&VW::get_unused_example(vw));
  }

  // need to keep the fb buffer around in order to process the event
  std::vector<flatbuffers::DetachedBuffer> obs_detached_buffers;

  auto observation_buffer =
      read_file(input_files + "/fb_events/fi-reward_v2.fb");
  joined_ccb_events =
      wrap_into_joined_events(observation_buffer, obs_detached_buffers);

  for (auto &je : joined_ccb_events) {
    joiner.process_event(*je);
  }

  BOOST_CHECK_EQUAL(joiner.processing_batch(), true);

  joiner.process_joined(examples);
  // learn/predict isn't called in the unit test but cleanup examples expects
  // shared pred to be set
  examples[0]->pred.decision_scores = {v_init<ACTION_SCORE::action_score>()};
  examples[0]->pred.decision_scores[0].push_back({0, 0.f});

  // shared, two actions, two slots and one empty (end of multiline)
  BOOST_CHECK_EQUAL(examples.size(), 6);
  // first example is shared example
  BOOST_CHECK_EQUAL(CCB::ec_is_example_header(*examples[0]), true);
  // next two examples are actions
  BOOST_CHECK_EQUAL(examples[1]->l.conditional_contextual_bandit.type,
                    CCB::example_type::action);
  BOOST_CHECK_EQUAL(examples[2]->l.conditional_contextual_bandit.type,
                    CCB::example_type::action);
  // next two examples are slots
  BOOST_CHECK_EQUAL(examples[3]->l.conditional_contextual_bandit.type,
                    CCB::example_type::slot);
  BOOST_CHECK_EQUAL(examples[4]->l.conditional_contextual_bandit.type,
                    CCB::example_type::slot);
  // last example is empty
  BOOST_CHECK_EQUAL(example_is_newline(*examples[5]), true);

  // check slot labels
  BOOST_CHECK_EQUAL(
      examples[3]
          ->l.conditional_contextual_bandit.outcome->probabilities.size(),
      2);
  BOOST_CHECK_EQUAL(
      examples[4]
          ->l.conditional_contextual_bandit.outcome->probabilities.size(),
      1);

  // check example features
  // from example_gen payload is JSON_CCB_CONTEXT =
  // R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a1":"f1"}}]})";

  BOOST_CHECK_EQUAL(examples[0]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[0]->indices[0], 'G');
  BOOST_CHECK_EQUAL(examples[1]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[1]->indices[0], 'T');
  BOOST_CHECK_EQUAL(examples[2]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[2]->indices[0], 'T');
  BOOST_CHECK_EQUAL(examples[3]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[3]->indices[0], 'S');
  BOOST_CHECK_EQUAL(examples[4]->indices.size(), 1);
  BOOST_CHECK_EQUAL(examples[4]->indices[0], 'S');
  BOOST_CHECK_EQUAL(examples[5]->indices.size(), 0); // newline example

  BOOST_CHECK_EQUAL(joiner.processing_batch(), false);
  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(example_joiner_test_joiner_ready) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);

  //the initial state of the joiner is not ready
  BOOST_CHECK_EQUAL(joiner.joiner_ready(), false);

  //all values but problem type have a default value
  joiner.set_problem_type_config(v2::ProblemType_CB);

  BOOST_CHECK_EQUAL(joiner.joiner_ready(), true);
}

BOOST_AUTO_TEST_CASE(example_joiner_test_stickyness) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);

  BOOST_CHECK_EQUAL(joiner.joiner_ready(), false);

  //all values but problem type have a default value
  joiner.set_problem_type_config(v2::ProblemType_CB, false);
  BOOST_CHECK_EQUAL(joiner.problem_type_config(), v2::ProblemType_CB);

  joiner.set_problem_type_config(v2::ProblemType_SLATES, true);
  BOOST_CHECK_EQUAL(joiner.problem_type_config(), v2::ProblemType_SLATES);

  // once the value is sticky, it cannot be modified
  joiner.set_problem_type_config(v2::ProblemType_CB, false);
  BOOST_CHECK_EQUAL(joiner.problem_type_config(), v2::ProblemType_SLATES);

  joiner.set_problem_type_config(v2::ProblemType_CB, true);
  BOOST_CHECK_EQUAL(joiner.problem_type_config(), v2::ProblemType_SLATES);
}
