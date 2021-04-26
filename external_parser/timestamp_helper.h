#pragma once

#include "generated/v2/Metadata_generated.h"

// rlclientlib headers
#include "time_helper.h"
// rlclientlub headers

namespace v2 = reinforcement_learning::messages::flatbuff::v2;
namespace rl = reinforcement_learning;

rl::timestamp to_timestamp(const v2::TimeStamp &ts);

rl::timestamp max_timestamp();

bool first_smaller_than_second(const rl::timestamp &a, const rl::timestamp &b);