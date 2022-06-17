#include "mock_helpers.h"

#include "constants.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;

std::unique_ptr<fakeit::Mock<r::i_sender>> get_mock_sender(int send_return_code)
{
  auto mock = std::unique_ptr<fakeit::Mock<r::i_sender>>(new fakeit::Mock<r::i_sender>());

  fakeit::When(Method((*mock), init)).AlwaysReturn(r::error_code::success);
  fakeit::When(Method((*mock), send)).AlwaysReturn(send_return_code);
  fakeit::Fake(Dtor((*mock)));

  return mock;
}

std::unique_ptr<r::sender_factory_t> get_mock_sender_factory(
    fakeit::Mock<r::i_sender>* mock_observation_sender, fakeit::Mock<r::i_sender>* mock_interaction_sender)
{
  auto factory = std::unique_ptr<r::sender_factory_t>(new r::sender_factory_t());
  factory->register_type(r::value::OBSERVATION_EH_SENDER,
      [mock_observation_sender](r::i_sender** retval, const u::configuration&, r::error_callback_fn* error_callback,
          r::i_trace*, r::api_status*)
      {
        *retval = &mock_observation_sender->get();
        return r::error_code::success;
      });
  factory->register_type(r::value::INTERACTION_EH_SENDER,
      [mock_interaction_sender](r::i_sender** retval, const u::configuration&, r::error_callback_fn* error_callback,
          r::i_trace*, r::api_status*)
      {
        *retval = &mock_interaction_sender->get();
        return r::error_code::success;
      });
  return factory;
}