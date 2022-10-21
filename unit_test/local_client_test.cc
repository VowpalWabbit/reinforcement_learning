#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "err_constants.h"
#include "factory_resolver.h"
#include "federation/federated_client.h"
#include "federation/local_client.h"
#include "vw/config/options.h"
#include "vw/config/options_cli.h"
#include "vw/core/io_buf.h"
#include "vw/core/merge.h"
#include "vw/core/parse_example.h"
#include "vw/core/parse_example_json.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"

#include <cstdio>
#include <memory>

namespace rl = reinforcement_learning;
namespace rerr = reinforcement_learning::error_code;
namespace rutil = reinforcement_learning::utility;

VW::multi_ex parse_json(VW::workspace& all, const std::string& line)
{
  VW::multi_ex examples;
  examples.push_back(&VW::get_unused_example(&all));
  VW::read_line_json_s<false>(
      all, examples, (char*)line.c_str(), line.length(), (VW::example_factory_t)&VW::get_unused_example, (void*)&all);
  VW::setup_examples(all, examples);
  return examples;
}

BOOST_AUTO_TEST_CASE(get_model_twice_fails)
{
  rutil::configuration config;
  std::unique_ptr<rl::i_federated_client> client;
  BOOST_CHECK_EQUAL(rl::create_local_client(config, client, nullptr, nullptr), rerr::success);
  rl::model_management::model_data data;
  bool model_received = false;
  BOOST_CHECK_EQUAL(client->try_get_model("test_app_id", data, model_received), rerr::success);
  BOOST_CHECK(data.data_sz() > 0);
  BOOST_CHECK_EQUAL(model_received, true);
  BOOST_CHECK_NE(client->try_get_model("test_app_id", data, model_received), rerr::success);
}

BOOST_AUTO_TEST_CASE(send_delta_update)
{
  rutil::configuration config;
  std::unique_ptr<rl::i_federated_client> client;
  BOOST_CHECK_EQUAL(rl::create_local_client(config, client, nullptr, nullptr), rerr::success);
  BOOST_CHECK_NE(client.get(), nullptr);
  rl::model_management::model_data data;
  bool model_received = false;
  BOOST_CHECK_EQUAL(client->try_get_model("test_app_id", data, model_received), rerr::success);
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{}));
  auto original_workspace =
      VW::initialize_experimental(std::move(opts), VW::io::create_buffer_view(data.data(), data.data_sz()));
  opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(std::vector<std::string>{}));
  auto workspace =
      VW::initialize_experimental(std::move(opts), VW::io::create_buffer_view(data.data(), data.data_sz()));
  std::string json_text = R"(
    {
      "s_": "1",
      "s_": "2",
      "_labelIndex": 0,
      "_label_Action": 1,
      "_label_Cost": 1,
      "_label_Probability": 0.5,
      "_multi": [
        {
          "a_": "1",
          "b_": "1",
          "c_": "1"
        },
        {
          "a_": "2",
          "b_": "2",
          "c_": "2"
        },
        {
          "a_": "3",
          "b_": "3",
          "c_": "3"
        }
      ]
    })";

  auto examples = parse_json(*workspace, json_text);
  workspace->learn(examples);
  workspace->finish_example(examples);

  auto delta = *workspace - *original_workspace;
  auto backing_buffer = std::make_shared<std::vector<char>>();
  auto writer = VW::io::create_vector_writer(backing_buffer);
  delta.serialize(*writer);
  BOOST_CHECK_EQUAL(
      client->report_result(reinterpret_cast<const uint8_t*>(backing_buffer->data()), backing_buffer->size()),
      rerr::success);
  BOOST_CHECK_NE(
      client->report_result(reinterpret_cast<const uint8_t*>(backing_buffer->data()), backing_buffer->size()),
      rerr::success);

  model_received = false;
  BOOST_CHECK_EQUAL(client->try_get_model("test_app_id", data, model_received), rerr::success);
}
