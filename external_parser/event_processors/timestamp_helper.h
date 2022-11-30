#pragma once

#include "date.h"
#include "generated/v2/Metadata_generated.h"
#include "vw/io/logger.h"

#include <chrono>

using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
TimePoint timestamp_to_chrono(const reinforcement_learning::messages::flatbuff::v2::TimeStamp& ts);
bool is_empty_timestamp(const reinforcement_learning::messages::flatbuff::v2::TimeStamp& ts);
TimePoint get_enqueued_time(const reinforcement_learning::messages::flatbuff::v2::TimeStamp* enqueued_time_utc,
    const reinforcement_learning::messages::flatbuff::v2::TimeStamp* client_time_utc, bool use_client_time,
    VW::io::logger& logger);