#include "test_common.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(cb_simple) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/valid_joined_logs/cb_simple.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  set_buffer_as_vw_input(buffer, vw);

  bool read_payload = false;
  while (vw->example_parser->reader(vw, examples) > 0) {
    read_payload = true;
    BOOST_CHECK_EQUAL(examples.size(), 4);
    BOOST_CHECK_EQUAL(examples[0]->indices.size(), 1);
    BOOST_CHECK_EQUAL(examples[0]->indices[0], 'G');
    BOOST_CHECK_EQUAL(examples[1]->indices.size(), 1);
    BOOST_CHECK_EQUAL(examples[1]->indices[0], 'T');
    BOOST_CHECK_EQUAL(examples[2]->indices.size(), 1);
    BOOST_CHECK_EQUAL(examples[2]->indices[0], 'T');
    BOOST_CHECK_EQUAL(examples[3]->indices.size(), 0); // newline example

    // simulate next call to parser->read by clearing up examples
    // and preparing one unused example
    clear_examples(examples, vw);
    examples.push_back(&VW::get_unused_example(vw));
  }

  BOOST_CHECK_EQUAL(read_payload, true);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(cb_dedup_compressed) {
  std::string input_files = get_test_files_location();

  auto buffer =
      read_file(input_files + "/valid_joined_logs/cb_dedup_compressed.log");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));
  set_buffer_as_vw_input(buffer, vw);

  bool read_payload = false;
  while (vw->example_parser->reader(vw, examples) > 0) {
    read_payload = true;
    BOOST_CHECK_EQUAL(examples.size(), 4);
    BOOST_CHECK_EQUAL(examples[0]->indices.size(), 1);
    BOOST_CHECK_EQUAL(examples[0]->indices[0], 'G');
    BOOST_CHECK_EQUAL(examples[1]->indices.size(), 1);
    BOOST_CHECK_EQUAL(examples[1]->indices[0], 'T');
    BOOST_CHECK_EQUAL(examples[2]->indices.size(), 1);
    BOOST_CHECK_EQUAL(examples[2]->indices[0], 'T');
    BOOST_CHECK_EQUAL(examples[3]->indices.size(), 0); // newline example

    // simulate next call to parser->read by clearing up examples
    // and preparing one unused example
    clear_examples(examples, vw);
    examples.push_back(&VW::get_unused_example(vw));
  }

  BOOST_CHECK_EQUAL(read_payload, true);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(compare_dsjson_with_fb_models) {
  std::string input_files = get_test_files_location();

  std::string fb_model = input_files + "/test_outputs/m_average_fb.model";
  std::string dsjson_model =
      input_files + "/test_outputs/m_average_dsjson.model";

  std::remove(fb_model.c_str());
  std::remove(dsjson_model.c_str());

  {
    // run with flatbuffer joined logs
    auto full_file_name =
        input_files + "/valid_joined_logs/average_reward_100_interactions.fb";

    auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet -f " +
                                 fb_model + " -d " + full_file_name,
                             nullptr, false, nullptr, nullptr);

    VW::start_parser(*vw);
    VW::LEARNER::generic_driver(*vw);
    VW::end_parser(*vw);

    VW::finish(*vw);
  }

  {
    // run with json joined logs
    auto full_file_name =
        input_files + "/valid_joined_logs/average_reward_100_interactions.json";

    auto vw = VW::initialize("--cb_explore_adf --dsjson --quiet -f " +
                                 dsjson_model + " -d " + full_file_name,
                             nullptr, false, nullptr, nullptr);

    VW::start_parser(*vw);
    VW::LEARNER::generic_driver(*vw);
    VW::end_parser(*vw);

    VW::finish(*vw);
  }

  // read the models and compare
  auto buffer_fb_model = read_file(fb_model);
  auto buffer_dsjson_model = read_file(dsjson_model);

  BOOST_CHECK_EQUAL_COLLECTIONS(buffer_fb_model.begin(), buffer_fb_model.end(),
                                buffer_dsjson_model.begin(),
                                buffer_dsjson_model.end());
}