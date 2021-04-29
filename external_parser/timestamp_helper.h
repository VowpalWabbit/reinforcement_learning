#pragma once

#include "generated/v2/Metadata_generated.h"
#include "date.h"

#include <chrono>

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

std::chrono::time_point<std::chrono::system_clock>
timestamp_to_chrono(const v2::TimeStamp &ts);