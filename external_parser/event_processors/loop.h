#pragma once

// FileFormat_generated.h used for the payload type and encoding enum's
#include "generated/v2/FileFormat_generated.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace loop {
struct loop_info {
  float default_reward = 0.f;
  v2::RewardFunctionType type = v2::RewardFunctionType_Earliest;
  v2::LearningModeType learning_mode_config = v2::LearningModeType_Online;
  v2::ProblemType problem_type_config = v2::ProblemType_UNKNOWN;
};
} // namespace loop