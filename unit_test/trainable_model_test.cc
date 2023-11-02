#include <boost/test/unit_test.hpp>

#include "common_test_utils.h"
#include "constants.h"
#include "err_constants.h"
#include "federation/vw_trainable_model.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <memory>

using namespace reinforcement_learning;

void setup_config(utility::configuration& config)
{
  config.set(name::PROTOCOL_VERSION, "2");
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  config.set(name::JOINER_PROBLEM_TYPE, value::PROBLEM_TYPE_UNKNOWN);
  config.set(name::JOINER_LEARNING_MODE, value::LEARNING_MODE_ONLINE);
  config.set(name::JOINER_REWARD_FUNCTION, value::REWARD_FUNCTION_EARLIEST);
}

BOOST_AUTO_TEST_CASE(trainable_model_set_get_data)
{
  utility::configuration config;
  setup_config(config);
  const std::string command_line = config.get(name::MODEL_VW_INITIAL_COMMAND_LINE, "");
  auto vw = test_utils::create_vw(command_line);

  // learn on one example
  auto ex = VW::read_example(*vw, "1 | a");
  vw->learn(*ex);
  vw->finish_example(*ex);
  const auto example_count = vw->sd->weighted_labeled_examples;
  BOOST_CHECK_EQUAL(example_count, 1.f);

  // put the workspace into trainable_vw_model
  std::unique_ptr<trainable_vw_model> model;
  BOOST_CHECK_EQUAL(trainable_vw_model::create(model, config), error_code::success);
  BOOST_CHECK_EQUAL(model->set_model(std::move(vw)), error_code::success);

  // get data out and check that it's equal
  model_management::model_data data_out;
  BOOST_CHECK_EQUAL(model->get_data(data_out), error_code::success);
  auto vw_out = test_utils::create_vw(command_line, data_out);
  BOOST_CHECK_EQUAL(vw_out->sd->weighted_labeled_examples, example_count);
}

BOOST_AUTO_TEST_CASE(trainable_model_learn_and_create_delta)
{
  const std::string command_line = "--quiet --preserve_performance_counters";

  // create 2 copies of the base VW workspace
  auto vw1 = test_utils::create_vw(command_line);
  auto vw2 = test_utils::create_vw(command_line);

  // learn on one example
  VW::example* ex = VW::read_example(*vw1, "1 | a");
  vw1->learn(*ex);
  vw1->finish_example(*ex);
  ex = VW::read_example(*vw2, "1 | a");
  vw2->learn(*ex);
  vw2->finish_example(*ex);

  // put the workspace into trainable_vw_model
  std::unique_ptr<trainable_vw_model> trainable_model;
  utility::configuration config;
  config.set(name::PROTOCOL_VERSION, "2");
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE, command_line.c_str());
  BOOST_CHECK_EQUAL(trainable_vw_model::create(trainable_model, config), error_code::success);
  BOOST_CHECK_EQUAL(trainable_model->set_model(std::move(vw1)), error_code::success);

  // train the trainable_vw_model on another example
  auto vw3 = test_utils::create_vw(command_line);
  std::vector<VW::example*> examples;
  examples.push_back(VW::read_example(*vw3, "1 | b"));
  BOOST_CHECK_EQUAL(trainable_model->learn(*vw3, examples), error_code::success);
  vw3->finish_example(*examples.back());

  // get data in trainable model
  model_management::model_data data_out;
  BOOST_CHECK_EQUAL(trainable_model->get_data(data_out), error_code::success);
  auto vw1_updated = test_utils::create_vw(command_line, data_out);

  // get model delta and update vw2 workspace
  VW::model_delta delta(nullptr);
  BOOST_CHECK_EQUAL(trainable_model->get_model_delta(delta), error_code::success);
  auto vw2_updated = *vw2 + delta;
  VW::workspace* delta_ws = delta.unsafe_get_workspace_ptr();
  BOOST_CHECK_EQUAL(delta_ws->sd->weighted_labeled_examples, 1.f);

  // check that results are same
  BOOST_CHECK_EQUAL(vw1_updated->sd->weighted_labeled_examples, 2.f);
  BOOST_CHECK_EQUAL(vw2_updated->sd->weighted_labeled_examples, 2.f);
}
