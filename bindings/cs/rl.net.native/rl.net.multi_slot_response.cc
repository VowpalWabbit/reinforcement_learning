#include "rl.net.multi_slot_response.h"

class multi_slot_enumerator_adapter
{
private:
    reinforcement_learning::multi_slot_response::const_iterator_t current;
    reinforcement_learning::multi_slot_response::const_iterator_t end;

public:
    inline multi_slot_enumerator_adapter(const reinforcement_learning::multi_slot_response* response) : current{response->begin()}, end{response->end()}
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

    inline reinforcement_learning::slot_entry const* operator*() const
    {
        return &*this->current;
    }
};

API int GetSlotEntrySlotId(reinforcement_learning::slot_entry* slot)
{
  return slot->get_slot_id();
}

API int GetSlotEntryActionId(reinforcement_learning::slot_entry* slot)
{
  return slot->get_action_id();
}

API float GetSlotEntryProbability(reinforcement_learning::slot_entry* slot)
{
  return slot->get_probability();
}

API reinforcement_learning::multi_slot_response* CreateMultiSlotResponse()
{
    return new reinforcement_learning::multi_slot_response();
};

API void DeleteMultSlotResponse(reinforcement_learning::multi_slot_response* multi_slot)
{
    delete multi_slot;
}

API size_t GetMultiSlotSize(reinforcement_learning::multi_slot_response* multi_slot)
{
    return multi_slot->size();
}

API const char* GetMultiSlotModelId(reinforcement_learning::multi_slot_response* multi_slot)
{
    return multi_slot->get_model_id();
}

API const char* GetMultSlotEventId(reinforcement_learning::multi_slot_response* multi_slot)
{
    return multi_slot->get_event_id();
}

API multi_slot_enumerator_adapter* CreateMultiSlotEnumeratorAdapter(reinforcement_learning::multi_slot_response* multi_slot)
{
    return new multi_slot_enumerator_adapter(multi_slot);
}

API void DeleteMultSlotEnumeratorAdapter(multi_slot_enumerator_adapter* adapter)
{
    delete adapter;
}

API int MultSlotEnumeratorInit(multi_slot_enumerator_adapter* adapter)
{
    return adapter->check_current();
}

API int MultSlotEnumeratorMoveNext(multi_slot_enumerator_adapter* adapter)
{
    return adapter->move_next();
}

API reinforcement_learning::slot_entry const* GetMultiSlotEnumeratorCurrent(multi_slot_enumerator_adapter* adapter)
{
    return adapter->operator*();
}
