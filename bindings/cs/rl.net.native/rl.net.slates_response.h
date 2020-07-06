#pragma once

#include "rl.net.native.h"

// TODO: Make the underlying iterator more ammenable to P/Invoke projection
class slates_enumerator_adapter;

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API int GetSlatesSlotSlotId(reinforcement_learning::slates_slot_response* slot);
    API int GetSlatesSlotActionId(reinforcement_learning::slates_slot_response* slot);
    API float GetSlatesSlotProbability(reinforcement_learning::slates_slot_response* slot);

    API reinforcement_learning::slates_response* CreateSlatesResponse();
    API void DeleteSlatesResponse(reinforcement_learning::slates_response* slates);

    API size_t GetSlatesSize(reinforcement_learning::slates_response* slates);

    API const char* GetSlatesModelId(reinforcement_learning::slates_response* slates);
    API const char* GetSlatesEventId(reinforcement_learning::slates_response* slates);

    API slates_enumerator_adapter* CreateSlatesEnumeratorAdapter(reinforcement_learning::slates_response* slates);
    API void DeleteSlatesEnumeratorAdapter(slates_enumerator_adapter* adapter);

    API int SlatesEnumeratorInit(slates_enumerator_adapter* adapter);
    API int SlatesEnumeratorMoveNext(slates_enumerator_adapter* adapter);
    API reinforcement_learning::slates_slot_response const* GetSlatesEnumeratorCurrent(slates_enumerator_adapter* adapter);
}