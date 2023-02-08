#define BOOST_TEST_DYN_LINK

#include "mock_util.h"

#include "constants.h"
#include "err_constants.h"
#include "model_mgmt.h"
#include "ranking_response.h"

namespace r = reinforcement_learning;
namespace m = r::model_management;
namespace u = r::utility;

using namespace fakeit;

std::unique_ptr<fakeit::Mock<r::i_sender>> get_mock_sender(int send_return_code)
{
  auto mock = std::unique_ptr<Mock<r::i_sender>>(new fakeit::Mock<r::i_sender>());

  When(Method((*mock), init)).AlwaysReturn(r::error_code::success);
  When(Method((*mock), send)).AlwaysReturn(send_return_code);
  Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<fakeit::Mock<r::i_sender>> get_mock_sender(std::vector<buffer_data_t>& recorded_messages)
{
  auto mock = std::unique_ptr<Mock<r::i_sender>>(new fakeit::Mock<r::i_sender>());
  const std::function<int(const buffer_t&, r::api_status*&)> send_fn =

      [&recorded_messages](const buffer_t& message, r::api_status*& status)
  {
    // take a copy of the data
    // the 'message' is a shared pointer to an object on the object pool
    // if live_model dtor is called prior to the 'recorded_messages' vector (as is done in most of the test cases)
    // there will be an invalid attempt to restore/free the object to/from the destructed object pool
    recorded_messages.push_back(*message.get());
    return r::error_code::success;
  };
  When(Method((*mock), init)).AlwaysReturn(r::error_code::success);
  When(Method((*mock), send)).AlwaysDo(send_fn);
  Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<fakeit::Mock<m::i_data_transport>> get_mock_data_transport()
{
  auto mock = std::unique_ptr<Mock<m::i_data_transport>>(new fakeit::Mock<m::i_data_transport>());

  When(Method((*mock), get_data)).AlwaysReturn(r::error_code::success);
  Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<fakeit::Mock<m::i_data_transport>> get_mock_failing_data_transport()
{
  auto mock = std::unique_ptr<fakeit::Mock<m::i_data_transport>>(new fakeit::Mock<m::i_data_transport>());

  When(Method((*mock), get_data)).AlwaysReturn(r::error_code::exception_during_http_req);
  Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<fakeit::Mock<m::i_model>> get_mock_model(m::model_type_t model_type)
{
  auto mock = std::unique_ptr<Mock<m::i_model>>(new fakeit::Mock<m::i_model>());

  const auto choose_rank_fn = [](const char*, uint64_t, r::string_view, std::vector<int>&, std::vector<float>&,
                                  std::string& model_version, r::api_status*)
  {
    model_version = "model_id";
    return r::error_code::success;
  };

  const auto choose_continuous_action_fn =
      [](r::string_view, float&, float&, std::string& model_version, r::api_status*)
  {
    model_version = "model_id";
    return r::error_code::success;
  };

  const auto request_decision_fn = [](const std::vector<const char*>& event_ids, r::string_view,
                                       std::vector<std::vector<uint32_t>>&, std::vector<std::vector<float>>&,
                                       std::string& model_version, r::api_status*)
  {
    model_version = "model_id";
    return r::error_code::success;
  };

  const auto request_multi_slot_decision_fn = [](const char*, const std::vector<std::string>&, r::string_view,
                                                  std::vector<std::vector<uint32_t>>&, std::vector<std::vector<float>>&,
                                                  std::string& model_version, r::api_status*)
  {
    model_version = "model_id";
    return r::error_code::success;
  };

  const auto choose_rank_multistep_fn = [](const char*, uint64_t, r::string_view, const r::episode_history&,
                                            std::vector<int>&, std::vector<float>&, std::string& model_version,
                                            r::api_status*)
  {
    model_version = "model_id";
    return r::error_code::success;
  };

  const auto get_model_type = [model_type]() { return model_type; };

  When(Method((*mock), update)).AlwaysReturn(r::error_code::success);
  When(Method((*mock), choose_rank)).AlwaysDo(choose_rank_fn);
  When(Method((*mock), choose_continuous_action)).AlwaysDo(choose_continuous_action_fn);
  When(Method((*mock), request_decision)).AlwaysDo(request_decision_fn);
  When(Method((*mock), request_multi_slot_decision)).AlwaysDo(request_multi_slot_decision_fn);
  When(Method((*mock), choose_rank_multistep)).AlwaysDo(choose_rank_multistep_fn);
  When(Method((*mock), model_type)).AlwaysDo(get_model_type);

  Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<r::sender_factory_t> get_mock_sender_factory(
    fakeit::Mock<r::i_sender>* mock_observation_sender, fakeit::Mock<r::i_sender>* mock_interaction_sender)
{
  auto factory = std::unique_ptr<r::sender_factory_t>(new r::sender_factory_t());
  factory->register_type(r::value::get_default_observation_sender(),
      [mock_observation_sender](std::unique_ptr<r::i_sender>& retval, const u::configuration&, r::error_callback_fn* error_callback,
          r::i_trace*, r::api_status*)
      {
        retval.reset(&mock_observation_sender->get());
        return r::error_code::success;
      });
  factory->register_type(r::value::get_default_interaction_sender(),
      [mock_interaction_sender](std::unique_ptr<r::i_sender>& retval, const u::configuration&, r::error_callback_fn* error_callback,
          r::i_trace*, r::api_status*)
      {
        retval.reset(&mock_interaction_sender->get());
        return r::error_code::success;
      });
  return factory;
}

std::unique_ptr<r::data_transport_factory_t> get_mock_data_transport_factory(
    fakeit::Mock<m::i_data_transport>* mock_data_transport)
{
  auto factory = std::unique_ptr<r::data_transport_factory_t>(new r::data_transport_factory_t());
  factory->register_type(r::value::get_default_data_transport(),
      [mock_data_transport](std::unique_ptr<m::i_data_transport>& retval, const u::configuration&, r::i_trace* trace, r::api_status*)
      {
        retval.reset(&mock_data_transport->get());
        return r::error_code::success;
      });
  return factory;
}

std::unique_ptr<r::model_factory_t> get_mock_model_factory(fakeit::Mock<m::i_model>* mock_model)
{
  auto factory = std::unique_ptr<r::model_factory_t>(new r::model_factory_t());
  factory->register_type(r::value::VW,
      [mock_model](std::unique_ptr<m::i_model>& retval, const u::configuration&, r::i_trace* trace, r::api_status*)
      {
        retval.reset(&mock_model->get());
        return r::error_code::success;
      });
  return factory;
}
