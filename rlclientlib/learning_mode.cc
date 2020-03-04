#pragma once
#include "learning_mode.h"
#include "constants.h"
#include <cstring>

namespace reinforcement_learning {
  namespace learning {
    learning_mode to_learning_mode(const char* learning_mode)
    {
      if (std::strcmp(learning_mode, value::LEARNING_MODE_IMITATION) == 0) {
        return IMITATION_MODE;
      }
      else if (std::strcmp(learning_mode, value::LEARNING_MODE_ONLINE) == 0) {
        return ONLINE_MODE;
      }
      else {
        return ONLINE_MODE;
      }
    }
  }
}
