#include "federation/vw_local_joiner.h"

#include "vw/config/options_cli.h"
#include "vw/core/vw.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace reinforcement_learning;

BOOST_AUTO_TEST_CASE(vw_joined_log_batch_test)
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
