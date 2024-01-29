#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace reinforcement_learning
{
using oauth_callback_t =
    std::function<int(std::string&, std::chrono::system_clock::time_point&, 
        const std::vector<std::string>&)>;
}