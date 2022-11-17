#include <boost/test/unit_test.hpp>

#include "configuration.h"
#include "constants.h"
#include "err_constants.h"
#include "federation/event_sink.h"
#include "federation/federated_client.h"
#include "federation/local_client.h"
#include "federation/local_loop_controller.h"
#include "federation/sender_joined_log_provider.h"

using namespace reinforcement_learning;

namespace
{
// Wrapper around local_loop_controller to allow us to construct it with custom member variables
class test_local_loop_controller : public local_loop_controller
{
public:
  test_local_loop_controller(std::string app_id, std::unique_ptr<i_federated_client>&& federated_client,
      std::unique_ptr<trainable_vw_model>&& trainable_model, std::shared_ptr<i_joined_log_provider>&& joiner,
      std::shared_ptr<i_event_sink>&& event_sink)
      : local_loop_controller(std::move(app_id), std::move(federated_client), std::move(trainable_model),
            std::move(joiner), std::move(event_sink))
  {
  }

  virtual ~test_local_loop_controller() = default;
};

// Mock version that simply stores and reads data
class mock_federated_client : public i_federated_client
{
public:
  virtual int try_get_model(const std::string& app_id, model_management::model_data& data, bool& model_received,
      api_status* status = nullptr) override
  {
    if (_has_data)
    {
      data = std::move(_data);
      _has_data = false;
      model_received = true;
    }
    else
    {
      model_received = false;
    }
    return error_code::success;
  }

  virtual int report_result(const uint8_t* payload, size_t size, api_status* status = nullptr) override
  {
    _result.clear();
    _result.reserve(size);
    _result.insert(_result.begin(), payload, payload + size);
    _has_result = true;
    return error_code::success;
  }

  virtual ~mock_federated_client() = default;

  void load_model_data(model_management::model_data data)
  {
    _data = std::move(data);
    _has_data = true;
  }

  std::vector<uint8_t> get_result()
  {
    if (_has_result)
    {
      _has_result = false;
      return std::move(_result);
    }
    return std::vector<uint8_t>();
  }

  model_management::model_data _data;
  bool _has_data = false;
  std::vector<uint8_t> _result;
  bool _has_result = false;
};

// Mock version that simply stores and reads data
class mock_event_sink : public i_event_sink
{
public:
  using buffer = std::shared_ptr<utility::data_buffer>;

  virtual int receive_events(const buffer& data, api_status* status = nullptr) override
  {
    _data = data;
    return error_code::success;
  }

  buffer get_latest_event() { return _data; }

  virtual ~mock_event_sink() = default;

  buffer _data;
};

utility::configuration get_test_config()
{
  utility::configuration config;
  config.set(name::PROTOCOL_VERSION, "2");
  config.set(name::EUD_DURATION, "0:0:0");
  return config;
}
}  // namespace

BOOST_AUTO_TEST_CASE(sender_factory_test)
{
  // create the local_loop_controller
  auto config = get_test_config();
  std::unique_ptr<i_federated_client> federated_client;
  std::unique_ptr<trainable_vw_model> trainable_model;
  std::unique_ptr<sender_joined_log_provider> sender_joiner;
  BOOST_CHECK_EQUAL(local_client::create(federated_client, config), error_code::success);
  BOOST_CHECK_EQUAL(trainable_vw_model::create(trainable_model, config), error_code::success);
  BOOST_CHECK_EQUAL(sender_joined_log_provider::create(sender_joiner, config), error_code::success);

  std::shared_ptr<i_joined_log_provider> joiner = std::move(sender_joiner);
  std::shared_ptr<mock_event_sink> test_event_sink = std::make_shared<mock_event_sink>();
  std::shared_ptr<i_event_sink> event_sink = test_event_sink;

  std::unique_ptr<local_loop_controller> test_llc(new test_local_loop_controller("test_app_id",
      std::move(federated_client), std::move(trainable_model), std::move(joiner), std::move(event_sink)));

  // create a sender
  auto sender_factory = test_llc->get_local_sender_factory();
  i_sender* sender_raw = nullptr;
  sender_factory(&sender_raw, config, nullptr, nullptr, nullptr);

  BOOST_CHECK_NE(sender_raw, nullptr);
  std::unique_ptr<i_sender> sender(sender_raw);

  // send some data
  const char* test_data = "Testing the sender...";
  auto test_data_len = std::char_traits<char>::length(test_data);

  std::shared_ptr<utility::data_buffer> buffer_in = std::make_unique<utility::data_buffer>(test_data_len);
  std::memcpy(buffer_in->raw_begin(), test_data, test_data_len);
  sender->send(buffer_in);

  // check the output
  auto buffer_out = test_event_sink->get_latest_event();
  BOOST_CHECK_EQUAL(std::memcmp(buffer_out->raw_begin(), test_data, test_data_len), 0);
}
