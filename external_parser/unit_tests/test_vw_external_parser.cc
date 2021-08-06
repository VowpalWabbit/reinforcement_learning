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

BOOST_AUTO_TEST_CASE(ccb_simple) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/valid_joined_logs/ccb_simple.log");

  auto vw = VW::initialize("--ccb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  set_buffer_as_vw_input(buffer, vw);

  bool read_payload = false;
  while (vw->example_parser->reader(vw, examples) > 0) {
    read_payload = true;
    BOOST_CHECK_EQUAL(examples.size(), 6);
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

    multi_ex multi_exs;
    for (auto *ex : examples) {
      multi_exs.push_back(ex);
    }
    vw->learn(multi_exs);

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

void generate_dsjson_and_fb_models(const std::string &model_name,
                                   const std::string &vw_args,
                                   const std::string &file_name) {
  std::string fb_model = model_name + ".fb";
  std::string dsjson_model = model_name + ".json";

  std::remove(fb_model.c_str());
  std::remove(dsjson_model.c_str());

  std::string fb_file = file_name + ".fb";
  std::string dsjson_file = file_name + ".json";

  {
    auto vw = VW::initialize(vw_args + " --binary_parser --quiet -f " +
                                 fb_model + " -d " + fb_file,
                             nullptr, false, nullptr, nullptr);
    VW::start_parser(*vw);
    VW::LEARNER::generic_driver(*vw);
    VW::end_parser(*vw);

    VW::finish(*vw);
  }

  {
    auto vw = VW::initialize(vw_args + " --dsjson --quiet -f " + dsjson_model +
                                 " -d " + dsjson_file,
                             nullptr, false, nullptr, nullptr);
    VW::start_parser(*vw);
    VW::LEARNER::generic_driver(*vw);
    VW::end_parser(*vw);

    VW::finish(*vw);
  }
}

BOOST_AUTO_TEST_CASE(cb_compare_dsjson_with_fb_models) {
  std::string input_files = get_test_files_location();

  std::string model_name = input_files + "/test_outputs/m_average";

  std::string file_name =
      input_files + "/valid_joined_logs/average_reward_100_interactions";

  generate_dsjson_and_fb_models(model_name, "--cb_explore_adf ", file_name);

  // read the models and compare
  auto buffer_fb_model = read_file(model_name + ".fb");
  auto buffer_dsjson_model = read_file(model_name + ".json");

  BOOST_CHECK_EQUAL_COLLECTIONS(buffer_fb_model.begin(), buffer_fb_model.end(),
                                buffer_dsjson_model.begin(),
                                buffer_dsjson_model.end());
}

BOOST_AUTO_TEST_CASE(ccb_compare_dsjson_with_fb_models) {
  std::string input_files = get_test_files_location();

  std::string model_name = input_files + "/test_outputs/ccb_m_sum";

  std::string file_name =
      input_files + "/valid_joined_logs/ccb_sum_reward_100_interactions";

  generate_dsjson_and_fb_models(model_name, "--ccb_explore_adf ", file_name);

  // read the models and compare
  auto buffer_fb_model = read_file(model_name + ".fb");
  auto buffer_dsjson_model = read_file(model_name + ".json");

  BOOST_CHECK_EQUAL_COLLECTIONS(buffer_fb_model.begin(), buffer_fb_model.end(),
                                buffer_dsjson_model.begin(),
                                buffer_dsjson_model.end());
}

BOOST_AUTO_TEST_CASE(rrcr_ignore_examples_before_checkpoint) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/valid_joined_logs/rrcr.fb");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  set_buffer_as_vw_input(buffer, vw);

  int count = 0;
  while (vw->example_parser->reader(vw, examples) > 0) {
    ++count;
    clear_examples(examples, vw);
    examples.push_back(&VW::get_unused_example(vw));
  }

  BOOST_CHECK_EQUAL(count, 1);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(rcrrmr_file_magic_and_header_in_the_middle_works) {
  std::string input_files = get_test_files_location();

  auto buffer = read_file(input_files + "/valid_joined_logs/rcrrmr.fb");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  set_buffer_as_vw_input(buffer, vw);

  int count = 0;
  while (vw->example_parser->reader(vw, examples) > 0) {
    ++count;
    clear_examples(examples, vw);
    examples.push_back(&VW::get_unused_example(vw));
  }

  BOOST_CHECK_EQUAL(count, 3);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(cb_apprentice_mode) {
  std::string input_files = get_test_files_location();

  // this datafile contains 5 joined events. One joined event (the third joined
  // event)'s interaction does not match the baseline action and so the reward
  // will be the default reward The rest of the interactions match the baseline
  // action so the reward will not be the default reward
  auto buffer =
      read_file(input_files + "/valid_joined_logs/cb_apprentice_5.log");

  size_t event_without_baseline = 3;

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  set_buffer_as_vw_input(buffer, vw);

  size_t joined_events_count = 0;
  while (vw->example_parser->reader(vw, examples) > 0) {
    joined_events_count++;
    if (joined_events_count == event_without_baseline) {
      // non-baseline action, find the label and check that cost is -0.0 (i.e.
      // -1*default_reward)
      v_array<CB::cb_class> costs;
      for (auto *ex : examples) {
        if (ex->l.cb.costs.size() > 0 &&
            ex->l.cb.costs[0].probability != -1.f /*shared example*/) {
          // if cost has already been set and we have 2 costs then that's bad
          BOOST_CHECK_EQUAL(costs.size(), 0);
          // copy label's costs for examination
          costs = ex->l.cb.costs;
        }
      }
      // check this has actually been set
      BOOST_CHECK_EQUAL(costs.size(), 1);
      // check default cost
      BOOST_CHECK_EQUAL(costs[0].cost, -0.f);
    } else {
      // 1st example is the example with cost in the baseline case (since
      // baseline is 1)
      BOOST_CHECK_EQUAL(examples[1]->l.cb.costs.size(), 1);
      BOOST_CHECK_CLOSE(examples[1]->l.cb.costs[0].cost, -1.5f, FLOAT_TOL);
    }

    // simulate next call to parser->read by clearing up examples
    // and preparing one unused example
    clear_examples(examples, vw);
    examples.push_back(&VW::get_unused_example(vw));
  }

  // this file contains 5 joined events
  BOOST_CHECK_EQUAL(joined_events_count, 5);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(cb_skip_learn_w_activations_and_apprentice) {
  std::string input_files = get_test_files_location();

  // this datafile contains 10 joined events with the 5 first interactions being deferred actions
  // but 2 of those 5 have activations reported (event_id's: [e28a9ae6,bbf5c404])

  auto buffer =
      read_file(input_files + "/valid_joined_logs/cb_deferred_actions_w_activations_and_apprentice_10.fb");

  auto vw = VW::initialize("--cb_explore_adf --binary_parser --quiet", nullptr,
                           false, nullptr, nullptr);

  v_array<example *> examples;
  examples.push_back(&VW::get_unused_example(vw));

  set_buffer_as_vw_input(buffer, vw);

  size_t joined_events_count = 0;
  while (vw->example_parser->reader(vw, examples) > 0) {
    joined_events_count++;

    // simulate next call to parser->read by clearing up examples
    // and preparing one unused example
    clear_examples(examples, vw);
    examples.push_back(&VW::get_unused_example(vw));
  }

  // this file contains 10 joined events 5 deferred but 2 of those activated
  BOOST_CHECK_EQUAL(joined_events_count, 7);

  clear_examples(examples, vw);
  VW::finish(*vw);
}

BOOST_AUTO_TEST_CASE(cb_compare_dsjson_with_fb_models_deferred_actions_w_activations_and_apprentice) {
  std::string input_files = get_test_files_location();

  std::string model_name = input_files + "/test_outputs/deferred_actions_w_activations_and_apprentice";

  std::string file_name =
      input_files + "/valid_joined_logs/cb_deferred_actions_w_activations_and_apprentice_10";

  generate_dsjson_and_fb_models(model_name, "--cb_explore_adf ", file_name);

  // read the models and compare
  auto buffer_fb_model = read_file(model_name + ".fb");
  auto buffer_dsjson_model = read_file(model_name + ".json");

  BOOST_CHECK_EQUAL_COLLECTIONS(buffer_fb_model.begin(), buffer_fb_model.end(),
                                buffer_dsjson_model.begin(),
                                buffer_dsjson_model.end());
}
