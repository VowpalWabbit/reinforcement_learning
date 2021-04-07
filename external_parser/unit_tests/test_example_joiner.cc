#define BOOST_TEST_DYN_LINK

#include "example_joiner.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(example_joiner_test) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);
  v_array<example *> examples;

  std::string input_files = get_test_files_location();
  auto interaction_buffer = read_file(input_files + "/cb_v2.fb");
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

  auto observation_buffer = read_file(input_files + "/f-reward_v2.fb");
  joined_cb_events =
      wrap_into_joined_events(observation_buffer, obs_detached_buffers);

  for (auto &je : joined_cb_events) {
    joiner.process_event(*je);
  }

  BOOST_CHECK_EQUAL(joiner.processing_batch(), true);

  joiner.process_joined(examples);

  // TODO assert examples here

  BOOST_CHECK_EQUAL(joiner.processing_batch(), false);
  clear_examples(examples, vw);
  VW::finish(*vw);
}