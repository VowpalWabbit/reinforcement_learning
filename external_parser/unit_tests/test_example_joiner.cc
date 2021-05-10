#include "example_joiner.h"
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

  auto observation_buffer = read_file(input_files + "/fb_events/f-reward_v2.fb");
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