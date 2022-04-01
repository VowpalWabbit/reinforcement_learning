#include "rl.net.decision_response.h"

class decision_enumerator_adapter
{
private:
    reinforcement_learning::decision_response::const_iterator_t current;
    reinforcement_learning::decision_response::const_iterator_t end;

public:
    inline decision_enumerator_adapter(const reinforcement_learning::decision_response* response) : current{response->begin()}, end{response->end()}
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

    inline reinforcement_learning::slot_response const* operator*() const
    {
        return &*this->current;
    }
};

API const char* GetSlotSlotId(reinforcement_learning::slot_response* slot, int& slot_id_size)
{
    const auto slot_id = slot->get_slot_id();
    slot_id_size = static_cast<int>(slot_id.size());
    return slot_id.data();
}

API int GetSlotActionId(reinforcement_learning::slot_response* slot)
{
  return slot->get_action_id();
}

API float GetSlotProbability(reinforcement_learning::slot_response* slot)
{
  return slot->get_probability();
}

API reinforcement_learning::decision_response* CreateDecisionResponse()
{
    return new reinforcement_learning::decision_response();
};

API void DeleteDecisionResponse(reinforcement_learning::decision_response* decision)
{
    delete decision;
}

API size_t GetDecisionSize(reinforcement_learning::decision_response* decision)
{
    return decision->size();
}

// TODO: We should think about how to avoid extra string copies; ideally, err constants
// should be able to be shared between native/managed, but not clear if this is possible
// right now.
API const char* GetDecisionModelId(reinforcement_learning::decision_response* decision, int& model_id_size)
{
    const auto model_id = decision->get_model_id();
    model_id_size = static_cast<int>(model_id.size());
    return model_id.data();
}

API decision_enumerator_adapter* CreateDecisionEnumeratorAdapter(reinforcement_learning::decision_response* decision)
{
    return new decision_enumerator_adapter(decision);
}

API void DeleteDecisionEnumeratorAdapter(decision_enumerator_adapter* adapter)
{
    delete adapter;
}

API int DecisionEnumeratorInit(decision_enumerator_adapter* adapter)
{
    return adapter->check_current();
}

API int DecisionEnumeratorMoveNext(decision_enumerator_adapter* adapter)
{
    return adapter->move_next();
}

API reinforcement_learning::slot_response const* GetDecisionEnumeratorCurrent(decision_enumerator_adapter* adapter)
{
    return adapter->operator*();
}