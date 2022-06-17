#pragma once

#include "rl.net.native.h"

// Global exports
extern "C"
{
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API reinforcement_learning::api_status* CreateApiStatus();
  API void DeleteApiStatus(reinforcement_learning::api_status* status);

  // TODO: We should think about how to avoid extra string copies; ideally, err constants
  // should be able to be shared between native/managed, but not clear if this is possible
  // right now.
  API const char* GetApiStatusErrorMessage(reinforcement_learning::api_status* status);
  API int GetApiStatusErrorCode(reinforcement_learning::api_status* status);

  API void UpdateApiStatusSafe(reinforcement_learning::api_status* status, int error_code, const char* message);
  API void ClearApiStatusSafe(reinforcement_learning::api_status* status);
}
