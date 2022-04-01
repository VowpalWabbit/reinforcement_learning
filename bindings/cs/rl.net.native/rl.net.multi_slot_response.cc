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

API void DeleteMultiSlotResponse(reinforcement_learning::multi_slot_response* multi_slot)
{
    delete multi_slot;
}

API size_t GetMultiSlotSize(reinforcement_learning::multi_slot_response* multi_slot)
{
    return multi_slot->size();
}

API const char* GetMultiSlotModelId(reinforcement_learning::multi_slot_response* multi_slot, int& model_id_size)
{
    const auto model_id = multi_slot->get_model_id();
    model_id_size = static_cast<int>(model_id.size());
    return model_id.data();
}

API const char* GetMultiSlotEventId(reinforcement_learning::multi_slot_response* multi_slot, int& event_id_size)
{
    const auto event_id = multi_slot->get_event_id();
    event_id_size = static_cast<int>(event_id.size());
    return event_id.data();
}

API multi_slot_enumerator_adapter* CreateMultiSlotEnumeratorAdapter(reinforcement_learning::multi_slot_response* multi_slot)
{
    return new multi_slot_enumerator_adapter(multi_slot);
}

API void DeleteMultiSlotEnumeratorAdapter(multi_slot_enumerator_adapter* adapter)
{
    delete adapter;
}

API int MultiSlotEnumeratorInit(multi_slot_enumerator_adapter* adapter)
{
    return adapter->check_current();
}

API int MultiSlotEnumeratorMoveNext(multi_slot_enumerator_adapter* adapter)
{
    return adapter->move_next();
}

API reinforcement_learning::slot_entry const* GetMultiSlotEnumeratorCurrent(multi_slot_enumerator_adapter* adapter)
{
    return adapter->operator*();
}
