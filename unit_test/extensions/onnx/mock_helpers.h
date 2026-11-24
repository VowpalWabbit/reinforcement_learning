#pragma once

#include "factory_resolver.h"
#include "sender.h"

#ifdef __GNUG__

// Fakeit does not work with GCC's devirtualization
// which is enabled with -O2 (the default) or higher.
#  pragma GCC optimize("no-devirtualize")

#endif

#include <fakeit/fakeit.hpp>
#include <memory>

std::unique_ptr<fakeit::Mock<reinforcement_learning::i_sender>> get_mock_sender(int send_return_code);
std::unique_ptr<reinforcement_learning::sender_factory_t> get_mock_sender_factory(
    fakeit::Mock<reinforcement_learning::i_sender>* mock_observation_sender,
    fakeit::Mock<reinforcement_learning::i_sender>* mock_interaction_sender);