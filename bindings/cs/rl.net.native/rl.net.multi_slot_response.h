#pragma once

#include "rl.net.native.h"

// TODO: Make the underlying iterator more ammenable to P/Invoke projection
class multi_slot_enumerator_adapter;

// Global exports
extern "C" {
    // NOTE: THIS IS NOT POLYMORPHISM SAFE!
    API int GetSlotEntrySlotId(reinforcement_learning::slot_entry* slot);
    API int GetSlotEntryActionId(reinforcement_learning::slot_entry* slot);
    API float GetSlotEntryProbability(reinforcement_learning::slot_entry* slot);

    API reinforcement_learning::multi_slot_response* CreateMultiSlotResponse();
    API void DeleteMultiSlotResponse(reinforcement_learning::multi_slot_response* multi_slot);

    API size_t GetMultiSlotSize(reinforcement_learning::multi_slot_response* multi_slot);

    API const char* GetMultiSlotModelId(reinforcement_learning::multi_slot_response* multi_slot);
    API const char* GetMultiSlotEventId(reinforcement_learning::multi_slot_response* multi_slot);

    API multi_slot_enumerator_adapter* CreateMultiSlotEnumeratorAdapter(reinforcement_learning::multi_slot_response* multi_slot);
    API void DeleteMultiSlotEnumeratorAdapter(multi_slot_enumerator_adapter* adapter);

    API int MultiSlotEnumeratorInit(multi_slot_enumerator_adapter* adapter);
    API int MultiSlotEnumeratorMoveNext(multi_slot_enumerator_adapter* adapter);
    API reinforcement_learning::slot_entry const* GetMultiSlotEnumeratorCurrent(multi_slot_enumerator_adapter* adapter);
}