#include "rl.net.slates_response.h"

class slates_enumerator_adapter
{
private:
    reinforcement_learning::slates_response::const_iterator_t current;
    reinforcement_learning::slates_response::const_iterator_t end;

public:
    inline slates_enumerator_adapter(const reinforcement_learning::slates_response* response) : current{response->begin()}, end{response->end()}
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

    inline reinforcement_learning::slates_slot_response const* operator*() const
    {
        return &*this->current;
    }
};

API int GetSlatesSlotSlotId(reinforcement_learning::slates_slot_response* slot)
{
  return slot->get_slot_id();
}

API int GetSlatesSlotActionId(reinforcement_learning::slates_slot_response* slot)
{
  return slot->get_action_id();
}

API float GetSlatesSlotProbability(reinforcement_learning::slates_slot_response* slot)
{
  return slot->get_probability();
}

API reinforcement_learning::slates_response* CreateSlatesResponse()
{
    return new reinforcement_learning::slates_response();
};

API void DeleteSlatesResponse(reinforcement_learning::slates_response* slates)
{
    delete slates;
}

API size_t GetSlatesSize(reinforcement_learning::slates_response* slates)
{
    return slates->size();
}

API const char* GetSlatesModelId(reinforcement_learning::slates_response* slates)
{
    return slates->get_model_id();
}

API const char* GetSlatesEventId(reinforcement_learning::slates_response* slates)
{
    return slates->get_event_id();
}

API slates_enumerator_adapter* CreateSlatesEnumeratorAdapter(reinforcement_learning::slates_response* slates)
{
    return new slates_enumerator_adapter(slates);
}

API void DeleteSlatesEnumeratorAdapter(slates_enumerator_adapter* adapter)
{
    delete adapter;
}

API int SlatesEnumeratorInit(slates_enumerator_adapter* adapter)
{
    return adapter->check_current();
}

API int SlatesEnumeratorMoveNext(slates_enumerator_adapter* adapter)
{
    return adapter->move_next();
}

API reinforcement_learning::slates_slot_response const* GetSlatesEnumeratorCurrent(slates_enumerator_adapter* adapter)
{
    return adapter->operator*();
}
