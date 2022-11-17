#include <boost/test/unit_test.hpp>

#include "configuration.h"
#include "constants.h"
#include "err_constants.h"
#include "federation/event_sink.h"
#include "federation/federated_client.h"
#include "federation/local_client.h"
#include "federation/local_loop_controller.h"
#include "federation/sender_joined_log_provider.h"
#include "vw/config/options_cli.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"

using namespace reinforcement_learning;

namespace
{
// Wrapper around local_loop_controller to allow us to access member variables
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

  i_federated_client* get_client() { return _federated_client.get(); }
  trainable_vw_model* get_model() { return _trainable_model.get(); }
  i_joined_log_provider* get_joiner() { return _joiner.get(); }
  i_event_sink* get_event_sink() { return _event_sink.get(); }
};

// Mock version that simply stores and reads data
class mock_federated_client : public i_federated_client
{
public:
  virtual int try_get_model(const std::string& app_id, model_management::model_data& data, bool& model_received,
      api_status* status = nullptr) override
  {
    if (_need_to_report_result) return -1;
    if (_has_data)
    {
      data = std::move(_data);
      _has_data = false;
      model_received = true;
      _need_to_report_result = true;
    }
    else
      model_received = false;
    return error_code::success;
  }

  virtual int report_result(const uint8_t* payload, size_t size, api_status* status = nullptr) override
  {
    if (!_need_to_report_result) return -1;
    _result.clear();
    _result.reserve(size);
    _result.insert(_result.begin(), payload, payload + size);
    _has_result = true;
    _need_to_report_result = false;
    return error_code::success;
  }

  mock_federated_client() = default;
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
  bool _need_to_report_result = false;
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
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  config.set(name::PROTOCOL_VERSION, "2");
  config.set(name::EUD_DURATION, "0:0:0");
  return config;
}

std::unique_ptr<local_loop_controller> create_test_local_loop_controller(utility::configuration config)
{
  std::unique_ptr<trainable_vw_model> trainable_model;
  std::unique_ptr<sender_joined_log_provider> sender_joiner;
  BOOST_CHECK_EQUAL(trainable_vw_model::create(trainable_model, config), error_code::success);
  BOOST_CHECK_EQUAL(sender_joined_log_provider::create(sender_joiner, config), error_code::success);

  std::shared_ptr<i_joined_log_provider> joiner = std::move(sender_joiner);
  std::shared_ptr<i_event_sink> event_sink(new mock_event_sink());
  std::unique_ptr<i_federated_client> federated_client(new mock_federated_client());

  return std::unique_ptr<local_loop_controller>(new test_local_loop_controller("test_app_id",
      std::move(federated_client), std::move(trainable_model), std::move(joiner), std::move(event_sink)));
}
}  // namespace

BOOST_AUTO_TEST_CASE(sender_factory_test)
{
  // create the local_loop_controller
  auto config = get_test_config();
  auto test_llc = create_test_local_loop_controller(config);

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

  // get the data out of event sink
  auto event_sink_out = dynamic_cast<test_local_loop_controller*>(test_llc.get())->get_event_sink();
  BOOST_CHECK_NE(event_sink_out, nullptr);
  auto buffer_out = dynamic_cast<mock_event_sink*>(event_sink_out)->get_latest_event();
  BOOST_CHECK_NE(buffer_out, nullptr);
  BOOST_CHECK_EQUAL(std::memcmp(buffer_out->raw_begin(), test_data, test_data_len), 0);
}

BOOST_AUTO_TEST_CASE(update_get_model_data)
{
  // create the local_loop_controller
  auto config = get_test_config();
  auto test_llc = create_test_local_loop_controller(config);
  auto generic_client = dynamic_cast<test_local_loop_controller*>(test_llc.get())->get_client();
  BOOST_CHECK_NE(generic_client, nullptr);
  auto mock_client = dynamic_cast<mock_federated_client*>(generic_client);
  BOOST_CHECK_NE(mock_client, nullptr);

  // this should do nothing because there's no global model to retrieve yet
  BOOST_CHECK_EQUAL(test_llc->update_global(), error_code::success);

  // create a model and train on an example
  const std::string command_line = config.get(name::MODEL_VW_INITIAL_COMMAND_LINE, "");
  auto opts = std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(VW::split_command_line(command_line)));
  auto vw = VW::initialize_experimental(std::move(opts));
  auto ex = VW::read_example(*vw, "1 | a");
  vw->learn(*ex);
  vw->finish_example(*ex);

  // serialize the model for mock client to consume
  io_buf io_buffer;
  auto backing_buffer = std::make_shared<std::vector<char>>();
  io_buffer.add_file(VW::io::create_vector_writer(backing_buffer));
  VW::save_predictor(*vw, io_buffer);

  // this should update the internal model
  model_management::model_data data_in;
  auto* data_in_buffer = data_in.alloc(backing_buffer->size());
  std::memcpy(data_in_buffer, backing_buffer->data(), backing_buffer->size());
  mock_client->load_model_data(data_in);
  BOOST_CHECK_EQUAL(test_llc->update_global(), error_code::success);

  // check that data retrieved is the same as data provided
  model_management::model_data data_out;
  BOOST_CHECK_EQUAL(test_llc->get_data(data_out), error_code::success);
  BOOST_CHECK_EQUAL(data_in.data_sz(), data_out.data_sz());
  BOOST_CHECK_EQUAL(std::memcmp(data_in.data(), data_out.data(), data_in.data_sz()), 0);

  // this should generate a model delta
  BOOST_CHECK_EQUAL(test_llc->update_global(), error_code::success);
  auto serialized_delta = mock_client->get_result();
  BOOST_CHECK_NE(serialized_delta.size(), 0);

  // check that data has not changed
  BOOST_CHECK_EQUAL(test_llc->get_data(data_out), error_code::success);
  BOOST_CHECK_EQUAL(data_in.data_sz(), data_out.data_sz());
  BOOST_CHECK_EQUAL(std::memcmp(data_in.data(), data_out.data(), data_in.data_sz()), 0);

  // delta should do nothing since we didn't train on any new examples
  auto delta_reader =
      VW::io::create_buffer_view(reinterpret_cast<const char*>(serialized_delta.data()), serialized_delta.size());
  auto delta = VW::model_delta::deserialize(*delta_reader);
  auto vw_new = *vw + *delta;
  BOOST_CHECK_EQUAL(vw->sd->weighted_labeled_examples, vw_new->sd->weighted_labeled_examples);
}
