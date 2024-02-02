#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace reinforcement_learning
{
using oauth_callback_t =
    std::function<int(const std::vector<std::string>&, std::string&, std::chrono::system_clock::time_point&)>;
}