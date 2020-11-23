#pragma once

#include "rl.net.native.h"

// TODO: Make the underlying iterator more ammenable to P/Invoke projection
class multi_slot_detailed_enumerator_adapter;

// Global exports
extern "C" {
	// NOTE: THIS IS NOT POLYMORPHISM SAFE!
	API reinforcement_learning::multi_slot_response_detailed* CreateMultiSlotResponseDetailed();
	API void DeleteMultiSlotResponseDetailed(reinforcement_learning::multi_slot_response_detailed* multi_slot);

	API const char* GetMultiSlotDetailedEventId(reinforcement_learning::multi_slot_response_detailed* slot);
	API const char* GetMultiSlotDetailedModelId(reinforcement_learning::multi_slot_response_detailed* slot);

	API size_t GetMultiSlotDetailedSize(reinforcement_learning::multi_slot_response_detailed* multi_slot);

	API multi_slot_detailed_enumerator_adapter* CreateMultiSlotDetailedEnumeratorAdapter(reinforcement_learning::multi_slot_response_detailed* multi_slot);
	API void DeleteMultiSlotDetailedEnumeratorAdapter(multi_slot_detailed_enumerator_adapter* adapter);

	API int MultiSlotDetailedEnumeratorInit(multi_slot_detailed_enumerator_adapter* adapter);
	API int MultiSlotDetailedEnumeratorMoveNext(multi_slot_detailed_enumerator_adapter* adapter);
	API reinforcement_learning::slot_ranking const* GetMultiSlotDetailedEnumeratorCurrent(multi_slot_detailed_enumerator_adapter* adapter);
}