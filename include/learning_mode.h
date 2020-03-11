/**
* @brief decision_modes definition.
*
* @file decision_modes.h
* @author Chenxi Zhao
* @date 2020-02-13
*/
#pragma once

namespace reinforcement_learning {

  enum learning_mode {
    ONLINE = 0,
    IMITATION = 1,
  };

  namespace learning {
    learning_mode to_learning_mode(const char* learning_mode);
  }
}
