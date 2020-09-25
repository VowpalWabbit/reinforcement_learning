#pragma once

#include "factory_resolver.h"
#include "sender.h"
#include "model_mgmt.h"

#include <memory>

#ifdef __GNUG__

// Fakeit does not work with GCC's devirtualization
// which is enabled with -O2 (the default) or higher.
#pragma GCC optimize("no-devirtualize")

#endif

#include <fakeit/fakeit.hpp>

using buffer_t = std::shared_ptr <reinforcement_learning::utility::data_buffer>;

std::unique_ptr<fakeit::Mock<reinforcement_learning::i_sender>> get_mock_sender(int send_return_code);
std::unique_ptr<fakeit::Mock<reinforcement_learning::i_sender>> get_mock_sender(std::vector<buffer_t>& recorded_messages);

std::unique_ptr<fakeit::Mock<reinforcement_learning::model_management::i_data_transport>> get_mock_data_transport();
std::unique_ptr<fakeit::Mock<reinforcement_learning::model_management::i_data_transport>> get_mock_failing_data_transport();
std::unique_ptr<fakeit::Mock<reinforcement_learning::model_management::i_model>> get_mock_model(reinforcement_learning::model_management::model_type_t = reinforcement_learning::model_management::model_type_t::UNKNOWN);

std::unique_ptr<reinforcement_learning::sender_factory_t> get_mock_sender_factory(fakeit::Mock<reinforcement_learning::i_sender>* mock_observation_sender,
  fakeit::Mock<reinforcement_learning::i_sender>* mock_interaction_sender);
std::unique_ptr<reinforcement_learning::data_transport_factory_t> get_mock_data_transport_factory(fakeit::Mock<reinforcement_learning::model_management::i_data_transport>* mock_data_transport);
std::unique_ptr<reinforcement_learning::model_factory_t> get_mock_model_factory(fakeit::Mock<reinforcement_learning::model_management::i_model>* mock_model);