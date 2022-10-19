#pragma once

#include "rl.net.native.h"

// TODO: Make the underlying iterator more ammenable to P/Invoke projection
class decision_enumerator_adapter;

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API const char* GetSlotSlotId(reinforcement_learning::slot_response* slot);
    API int GetSlotActionId(reinforcement_learning::slot_response* slot);
    API float GetSlotProbability(reinforcement_learning::slot_response* slot);

    API reinforcement_learning::decision_response* CreateDecisionResponse();
    API void DeleteDecisionResponse(reinforcement_learning::decision_response* decision);

    API size_t GetDecisionSize(reinforcement_learning::decision_response* decision);

    // TODO: We should think about how to avoid extra string copies; ideally, err constants
    // should be able to be shared between native/managed, but not clear if this is possible
    // right now.
    API const char* GetDecisionModelId(reinforcement_learning::decision_response* decision);

    API decision_enumerator_adapter* CreateDecisionEnumeratorAdapter(reinforcement_learning::decision_response* decision);
    API void DeleteDecisionEnumeratorAdapter(decision_enumerator_adapter* adapter);

    API int DecisionEnumeratorInit(decision_enumerator_adapter* adapter);
    API int DecisionEnumeratorMoveNext(decision_enumerator_adapter* adapter);
    API reinforcement_learning::slot_response const* GetDecisionEnumeratorCurrent(decision_enumerator_adapter* adapter);
}