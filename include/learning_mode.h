/**
* @brief learning_mode definition.
*
* @file learning_mode.h
* @author Chenxi Zhao
* @date 2020-02-13
*/
#pragma once
#include "vw/common/vwvis.h"

namespace reinforcement_learning {

  enum learning_mode {
    ONLINE = 0,
    APPRENTICE = 1,
    LOGGINGONLY = 2,
  };

  namespace learning {
    VW_DLL_PUBLIC learning_mode to_learning_mode(const char* learning_mode);
  }
}
