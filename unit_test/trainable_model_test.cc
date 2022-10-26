#include "federation/vw_trainable_model.h"
#include "federation/vw_local_joiner.h"
#include "constants.h"
#include "err_constants.h"
#include "vw/config/options_cli.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace reinforcement_learning;

BOOST_AUTO_TEST_CASE(trainable_model_get_data)
{
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{"--quiet", "--preserve_performance_counters"}));
  std::unique_ptr<VW::workspace> vw = VW::initialize_experimental(std::move(opts));

  // learn on one example
  auto ex = VW::read_example(*vw, "1 | a");
  vw->learn(*ex);
  vw->finish_example(*ex);
  const auto example_count = vw->sd->weighted_labeled_examples;
  BOOST_CHECK_EQUAL(example_count, 1.f);

  // put the workspace into trainable_vw_model
  utility::configuration config;
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  trainable_vw_model model(config);
  model.set_model(std::move(vw));

  // get data out and check that it's equal
  model_management::model_data data_out;
  model.get_data(data_out);
  opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{"--quiet", "--preserve_performance_counters"}));
  std::unique_ptr<VW::workspace> vw_out = VW::initialize_experimental(std::move(opts), VW::io::create_buffer_view(data_out.data(), data_out.data_sz()));
  BOOST_CHECK_EQUAL(vw_out->sd->weighted_labeled_examples, example_count);
}

BOOST_AUTO_TEST_CASE(trainable_model_learn_and_create_delta)
{
  // create 2 copies of the base VW workspace
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{"--quiet", "--preserve_performance_counters"}));
  std::unique_ptr<VW::workspace> vw1 = VW::initialize_experimental(std::move(opts));
  opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{"--quiet", "--preserve_performance_counters"}));
  std::unique_ptr<VW::workspace> vw2 = VW::initialize_experimental(std::move(opts));

  // learn on one example
  VW::example* ex = VW::read_example(*vw1, "1 | a");
  vw1->learn(*ex);
  vw1->finish_example(*ex);
  ex = VW::read_example(*vw2, "1 | a");
  vw2->learn(*ex);
  vw2->finish_example(*ex);

  // put the workspace into trainable_vw_model
  utility::configuration config;
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  trainable_vw_model model(config);
  model.set_model(std::move(vw1));

  // learn on another example
  opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{"--quiet", "--preserve_performance_counters"}));
  std::shared_ptr<VW::workspace> vw_for_joiner = VW::initialize_experimental(std::move(opts));
  vw_joined_log_batch batch(vw_for_joiner);
  ex = VW::read_example(*vw_for_joiner, "1 | b");
  batch.add_example(ex);
  model.learn(batch);

  // get data in trainable model
  model_management::model_data data_out;
  model.get_data(data_out);
  opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{"--quiet", "--preserve_performance_counters"}));
  std::unique_ptr<VW::workspace> vw1_updated = VW::initialize_experimental(std::move(opts), VW::io::create_buffer_view(data_out.data(), data_out.data_sz()));

  // get model delta and update vw2 workspace
  auto delta = model.get_model_delta();
  auto vw2_updated = *vw2 + delta;
  VW::workspace* delta_ws = delta.unsafe_get_workspace_ptr();
  BOOST_CHECK_EQUAL(delta_ws->sd->weighted_labeled_examples, 1.f);

  // check that results are same
  BOOST_CHECK_EQUAL(vw1_updated->sd->weighted_labeled_examples, 2.f);
  BOOST_CHECK_EQUAL(vw2_updated->sd->weighted_labeled_examples, 2.f);
}
