#include "rl.net.multi_slot_response_detailed.h"

class multi_slot_detailed_enumerator_adapter
{
private:
  reinforcement_learning::multi_slot_response_detailed::const_iterator_t current;
  reinforcement_learning::multi_slot_response_detailed::const_iterator_t end;

public:
  inline multi_slot_detailed_enumerator_adapter(const reinforcement_learning::multi_slot_response_detailed* response) : current{ response->begin() }, end{ response->end() }
  {}

public:
  inline int check_current()
  {
    if (this->current != this->end)
    {
      return 1;
    }

    return 0;
  }

  inline int move_next()
  {
    if ((this->current != this->end) &&
      ((++this->current) != this->end))
    {
      return 1;
    }

    return 0;
  }

  inline reinforcement_learning::slot_ranking const* operator*() const
  {
    return &*this->current;
  }
};


API reinforcement_learning::multi_slot_response_detailed* CreateMultiSlotResponseDetailed()
{
  return new reinforcement_learning::multi_slot_response_detailed();
};

API void DeleteMultiSlotResponseDetailed(reinforcement_learning::multi_slot_response_detailed* multi_slot)
{
  delete multi_slot;
}

API size_t GetMultiSlotDetailedSize(reinforcement_learning::multi_slot_response_detailed* multi_slot)
{
  return multi_slot->size();
}

API const char* GetMultiSlotDetailedModelId(reinforcement_learning::multi_slot_response_detailed* multi_slot, int& model_id_size)
{
  const auto model_id = multi_slot->get_model_id();
  model_id_size = model_id.size();
  return model_id.data();
}

API const char* GetMultiSlotDetailedEventId(reinforcement_learning::multi_slot_response_detailed* multi_slot, int& event_id_size)
{
  const auto event_id = multi_slot->get_event_id();
  event_id_size = event_id.size();
  return event_id.data();
}

API multi_slot_detailed_enumerator_adapter* CreateMultiSlotDetailedEnumeratorAdapter(reinforcement_learning::multi_slot_response_detailed* multi_slot)
{
  return new multi_slot_detailed_enumerator_adapter(multi_slot);
}

API void DeleteMultiSlotDetailedEnumeratorAdapter(multi_slot_detailed_enumerator_adapter* adapter)
{
  delete adapter;
}

API int MultiSlotDetailedEnumeratorInit(multi_slot_detailed_enumerator_adapter* adapter)
{
  return adapter->check_current();
}

API int MultiSlotDetailedEnumeratorMoveNext(multi_slot_detailed_enumerator_adapter* adapter)
{
  return adapter->move_next();
}

API reinforcement_learning::slot_ranking const* GetMultiSlotDetailedEnumeratorCurrent(multi_slot_detailed_enumerator_adapter* adapter)
{
  return adapter->operator*();
}
