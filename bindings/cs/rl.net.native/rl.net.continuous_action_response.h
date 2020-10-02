#pragma once

#include "rl.net.native.h"

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API reinforcement_learning::continuous_action_response* CreateContinuousActionResponse();
    API void DeleteContinuousActionResponse(reinforcement_learning::continuous_action_response* response);

    // TODO: We should think about how to avoid extra string copies; ideally, err constants
    // should be able to be shared between native/managed, but not clear if this is possible
    // right now.
    API const char* GetContinuousActionEventId(reinforcement_learning::continuous_action_response* response);
    API const char* GetContinuousActionModelId(reinforcement_learning::continuous_action_response* response);

    API float GetContinuousActionChosenAction(reinforcement_learning::continuous_action_response* response);
    API float GetContinuousActionChosenActionDensity(reinforcement_learning::continuous_action_response* response);
}