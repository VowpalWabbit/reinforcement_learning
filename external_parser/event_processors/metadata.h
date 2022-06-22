#pragma once

// FileFormat_generated.h used for the payload type and encoding enum's
#include "generated/v2/FileFormat_generated.h"
#include "timestamp_helper.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace metadata
{
// used both for interactions and observations
struct event_metadata_info
{
  std::string app_id;
  v2::PayloadType payload_type;
  float pass_probability;
  v2::EventEncoding event_encoding;
  std::string event_id;
  v2::LearningModeType learning_mode;
};
}  // namespace metadata
