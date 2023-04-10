#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "common_test_utils.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "federation/federated_client.h"
#include "federation/local_client.h"
#include "vw/core/merge.h"
#include "vw/core/parse_example.h"
#include "vw/core/parse_example_json.h"
#include "vw/core/vw.h"

#include <cstdio>
#include <memory>

using namespace reinforcement_learning;

VW::multi_ex parse_json(VW::workspace& all, const std::string& line)
{
  VW::multi_ex examples;
  examples.push_back(VW::new_unused_example(all));
  VW::example_factory_t ex_fac = [&all]() -> VW::example& { return *(VW::new_unused_example(all)); };
  VW::parsers::json::read_line_json<false>(all, examples, (char*)line.c_str(), line.length(), ex_fac);
  VW::setup_examples(all, examples);
  return examples;
}

BOOST_AUTO_TEST_CASE(get_model_twice_fails)
{
  utility::configuration config;
  std::unique_ptr<i_federated_client> client;
  BOOST_CHECK_EQUAL(local_client::create(client, config, nullptr, nullptr), error_code::success);
  model_management::model_data data;
  bool model_received = false;
  BOOST_CHECK_EQUAL(client->try_get_model("test_app_id", data, model_received), error_code::success);
  BOOST_CHECK(data.data_sz() > 0);
  BOOST_CHECK_EQUAL(model_received, true);
  BOOST_CHECK_NE(client->try_get_model("test_app_id", data, model_received), error_code::success);
}

BOOST_AUTO_TEST_CASE(send_delta_update)
{
  utility::configuration config;
  std::unique_ptr<i_federated_client> client;
  config.set("id", "test_app_id");
  BOOST_CHECK_EQUAL(local_client::create(client, config, nullptr, nullptr), error_code::success);
  BOOST_CHECK_NE(client.get(), nullptr);
  model_management::model_data data;
  bool model_received = false;
  BOOST_CHECK_EQUAL(client->try_get_model("test_app_id", data, model_received), error_code::success);

  auto original_workspace = test_utils::create_vw("", data);
  auto workspace = test_utils::create_vw("", data);
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
      error_code::success);
  BOOST_CHECK_NE(
      client->report_result(reinterpret_cast<const uint8_t*>(backing_buffer->data()), backing_buffer->size()),
      error_code::success);

  model_received = false;
  BOOST_CHECK_EQUAL(client->try_get_model("test_app_id", data, model_received), error_code::success);
}
