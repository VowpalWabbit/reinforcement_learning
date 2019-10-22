#pragma once
#include "api_status.h"

namespace reinforcement_learning {
  namespace utility {
    int check_null_or_empty(const char* arg1, const char* arg2, api_status* status);
    int check_null_or_empty(const char* arg1, api_status* status);
  }}
