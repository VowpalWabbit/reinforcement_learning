#include "learning_mode.h"
#include "constants.h"
#include <cstring>

// portability fun
#ifndef _WIN32
#define _stricmp strcasecmp
#endif

namespace reinforcement_learning {
  namespace learning {
    learning_mode to_learning_mode(const char* learning_mode)
    {
      if (_stricmp(learning_mode, value::LEARNING_MODE_IMITATION) == 0) {
        return IMITATION;
      }
      else if (_stricmp(learning_mode, value::LEARNING_MODE_ONLINE) == 0) {
        return ONLINE;
      }
      else {
        return ONLINE;
      }
    }
  }
}
