#define BOOST_TEST_DYN_LINK

#include "example_joiner.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(example_joiner_test) {
  auto vw = VW::initialize("--quiet --binary_parser --cb_explore_adf", nullptr,
                           false, nullptr, nullptr);

  example_joiner joiner(vw);

  std::string input_files = get_test_files_location();
  auto interaction_buffer = read_file(input_files + "/cb_v2.fb");
  // needed for memory management
  flatbuffers::DetachedBuffer int_detached_buffer;
  auto joined_cb_event =
      wrap_into_joined_event(interaction_buffer, int_detached_buffer);

  joiner.process_event(*joined_cb_event);

  // needed for memory management
  flatbuffers::DetachedBuffer obs_detached_buffer;
  auto observation_buffer = read_file(input_files + "/f-reward_v2.fb");
  joined_cb_event =
      wrap_into_joined_event(observation_buffer, obs_detached_buffer);

  joiner.process_event(*joined_cb_event);

  BOOST_CHECK_EQUAL(joiner.processing_batch(), true);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  joiner.process_joined(examples);

  // assert examples here

  BOOST_CHECK_EQUAL(joiner.processing_batch(), false);
  clear_examples(examples, vw);
  VW::finish(*vw);
}