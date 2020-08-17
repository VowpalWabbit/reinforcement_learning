#include "payload_serializer.h"

namespace reinforcement_learning {
  using namespace messages::flatbuff;
  namespace logger {
    v2::LearningModeType GetLearningMode(learning_mode mode) {
      v2::LearningModeType result;
      switch (mode) {
      case APPRENTICE: return v2::LearningModeType_Apprentice;
      case ONLINE: return v2::LearningModeType_Online;
      case LOGGINGONLY: return v2::LearningModeType_LoggingOnly;
      default: return v2::LearningModeType_Online;
      }
    }
  }
}