#include "joiners/example_joiner.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

void process(VW::workspace *vw, example_joiner &joiner, v_array<example *> &examples,
             std::string int_file, std::string obs_file) {
  std::string input_files = get_test_files_location();

  auto interaction_buffer = read_file(input_files + int_file);
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

  auto observation_buffer = read_file(input_files + obs_file);
  joined_cb_events =
      wrap_into_joined_events(observation_buffer, obs_detached_buffers);

  for (auto &je : joined_cb_events) {
    joiner.process_event(*je);
  }

  BOOST_CHECK_EQUAL(joiner.processing_batch(), true);

  joiner.process_joined(examples);
}

BOOST_AUTO_TEST_CASE(test_use_client_time_missing_and_configured_to_true) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);
  v_array<example *> examples;

  joiner.set_problem_type_config(v2::ProblemType_CB);
  // the test files used do not have a client time utc set, so they will use the
  // enqueued time utc even though this is set to true
  joiner.set_use_client_time(true);

  // rewards are {5, 4, 3}
  // enqueued time goes from latest to earliest
  // client time goes from earliest to latest
  float earliest_enqueued = -3.f;
  process(vw, joiner, examples, "/reward_functions/cb/cb_v2.fb",
          "/reward_functions/cb/f-reward_3obs_v2.fb");

  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].action, 1);
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].cost, earliest_enqueued);
  BOOST_CHECK_CLOSE(examples[1]->l.cb.costs[0].probability, 0.9, FLOAT_TOL);
  BOOST_CHECK_EQUAL(examples[1]->l.cb.weight, 1.0f);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_use_client_time_existing_and_configured_to_false) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);
  v_array<example *> examples;

  joiner.set_problem_type_config(v2::ProblemType_CB);
  // the test files used have a client time utc set, but enqueued time will be
  // used instead
  joiner.set_use_client_time(false);

  // rewards are {1, 1, 5}
  // enqueued time goes from latest to earliest
  // client time goes from earliest to latest
  float earliest_enqueued = -5.f;
  float earliest_client = -1.f;
  process(vw, joiner, examples, "/client_time/cb_v2_client_time.fb",
          "/client_time/f-reward_3obs_v2_client_time.fb");

  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].action, 1);
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].cost, earliest_enqueued);
  BOOST_CHECK_CLOSE(examples[1]->l.cb.costs[0].probability, 0.9, FLOAT_TOL);
  BOOST_CHECK_EQUAL(examples[1]->l.cb.weight, 1.0f);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(test_use_client_time_existing_and_configured_to_true) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);
  v_array<example *> examples;

  joiner.set_problem_type_config(v2::ProblemType_CB);
  // the test files used have a client time set and it should be used instead of
  // enqueued time utc
  joiner.set_use_client_time(true);

  // rewards are {1, 1, 5}
  // enqueued time goes from latest to earliest
  // client time goes from earliest to latest
  float earliest_enqueued = -5.f;
  float earliest_client = -1.f;
  process(vw, joiner, examples, "/client_time/cb_v2_client_time.fb",
          "/client_time/f-reward_3obs_v2_client_time.fb");

  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].action, 1);
  BOOST_CHECK_EQUAL(examples[1]->l.cb.costs[0].cost, earliest_client);
  BOOST_CHECK_CLOSE(examples[1]->l.cb.costs[0].probability, 0.9, FLOAT_TOL);
  BOOST_CHECK_EQUAL(examples[1]->l.cb.weight, 1.0f);

  clear_examples(examples, vw);
  VW::finish(*vw);
}