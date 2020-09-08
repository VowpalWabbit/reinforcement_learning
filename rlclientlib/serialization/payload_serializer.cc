#include "payload_serializer.h"

namespace reinforcement_learning {
  using namespace messages::flatbuff;
  namespace logger {
    int get_learning_mode(learning_mode mode_in, v2::LearningModeType& mode_out, api_status* status) {
      v2::LearningModeType result;
      switch (mode_in) {
        case APPRENTICE: mode_out = v2::LearningModeType_Apprentice; return error_code::success;
        case ONLINE: mode_out = v2::LearningModeType_Online; return error_code::success;
        case LOGGINGONLY: mode_out = v2::LearningModeType_LoggingOnly; return error_code::success;
        default: return report_error(status, error_code::unsupported_learning_mode, error_code::unsupported_learning_mode_s);
      }
    }
  }
}
