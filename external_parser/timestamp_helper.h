#pragma once

#include "date.h"
#include "generated/v2/Metadata_generated.h"

#include <chrono>

namespace v2 = reinforcement_learning::messages::flatbuff::v2;
using TimePoint =
    std::chrono::time_point<std::chrono::system_clock,
                            std::chrono::duration<int64_t, std::nano>>;
TimePoint timestamp_to_chrono(const v2::TimeStamp &ts);