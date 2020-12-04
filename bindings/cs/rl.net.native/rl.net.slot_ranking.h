#pragma once

#include "rl.net.native.h"

class slot_enumerator_adapter;

//Global Exports
extern "C" {
  //NOTE: THIS IS  NOT POLYMORPHISM SAFE
  API reinforcement_learning::slot_ranking* CreateSlotRanking();
  API void DeleteSlotRanking(reinforcement_learning::slot_ranking* slot);

  // TODO: We should think about how to avoid extra string copies; ideally, err constants
  // should be able to be shared between native/managed, but not clear if this is possible
  // right now.
  API const char* GetSlotId(reinforcement_learning::slot_ranking* slot);

  API size_t GetSlotActionCount(reinforcement_learning::slot_ranking* slot);

  API int GetSlotChosenAction(reinforcement_learning::slot_ranking* slot, size_t* action_id, reinforcement_learning::api_status* status = nullptr);

  API slot_enumerator_adapter* CreateSlotEnumeratorAdapter(reinforcement_learning::slot_ranking* slot);
  API void DeleteSlotEnumeratorAdapter(slot_enumerator_adapter* adapter);

  API int SlotEnumeratorInit(slot_enumerator_adapter* adapter);
  API int SlotEnumeratorMoveNext(slot_enumerator_adapter* adapter);
  API reinforcement_learning::action_prob_d GetSlotEnumeratorCurrent(slot_enumerator_adapter* adapter);
}