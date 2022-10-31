#include <boost/test/unit_test.hpp>

#include "federation/vw_local_joiner.h"
#include "vw/config/options_cli.h"
#include "vw/core/vw.h"

#include <memory>

using namespace reinforcement_learning;

BOOST_AUTO_TEST_CASE(vw_joined_log_batch_example_output)
{
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{"--quiet"}));
  std::shared_ptr<VW::workspace> vw = VW::initialize_experimental(std::move(opts));
  vw_joined_log_batch batch(vw);

  auto* ex1 = VW::read_example(*vw, "1 | a");
  auto* ex2 = VW::read_example(*vw, "1 | b");
  batch.add_example(ex1);
  batch.add_example(ex2);

  VW::example* example_out;

  // last in first out
  BOOST_CHECK_EQUAL(batch.next_example(&example_out), error_code::success);
  BOOST_CHECK_EQUAL(example_out, ex2);
  batch.finish_example(example_out);

  BOOST_CHECK_EQUAL(batch.next_example(&example_out), error_code::success);
  BOOST_CHECK_EQUAL(example_out, ex1);
  batch.finish_example(example_out);

  // should get nullptr when empty
  BOOST_CHECK_EQUAL(batch.next_example(&example_out), error_code::success);
  BOOST_CHECK_EQUAL(example_out, nullptr);
}

BOOST_AUTO_TEST_CASE(vw_joined_log_batch_binary_output)
{
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{"--quiet"}));
  std::shared_ptr<VW::workspace> vw = VW::initialize_experimental(std::move(opts));
  vw_joined_log_batch batch(vw);

  auto* ex = VW::read_example(*vw, "1 | a");
  batch.add_example(ex);

  // call next() to get binary buffer output
  std::unique_ptr<VW::io::reader> buffer_out;
  BOOST_CHECK_EQUAL(batch.next(buffer_out), error_code::success);

  // re-create the example object
  io_buf io_reader;
  io_reader.add_file(std::move(buffer_out));
  VW::multi_ex examples;
  examples.push_back(VW::new_unused_example(*vw));
  VW::read_example_from_cache(vw.get(), io_reader, examples);
  BOOST_CHECK_EQUAL(examples.size(), 1);
  BOOST_CHECK_NE(examples[0], nullptr);

  // check some fields inside the examples to make sure they're equal
  auto* ex_out = examples[0];
  VW::setup_example(*vw, ex);
  VW::setup_example(*vw, ex_out);
  BOOST_CHECK_EQUAL(ex_out->num_features, ex->num_features);
  BOOST_CHECK_EQUAL(ex_out->indices.size(), ex->indices.size());

  // should get nullptr when empty
  BOOST_CHECK_EQUAL(batch.next(buffer_out), error_code::success);
  BOOST_CHECK_EQUAL(buffer_out.get(), nullptr);

  // vw_joined_log_batch should automatically call finish_example on ex
  vw->finish_example(*ex_out);
}
