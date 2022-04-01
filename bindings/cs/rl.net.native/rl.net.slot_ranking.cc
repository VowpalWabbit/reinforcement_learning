#include "rl.net.slot_ranking.h"

class slot_enumerator_adapter
{
private:
  reinforcement_learning::slot_ranking::const_iterator current;
  reinforcement_learning::slot_ranking::const_iterator end;

public:
  inline slot_enumerator_adapter(const reinforcement_learning::slot_ranking* slot) : current{ slot->begin() }, end{ slot->end() }
  {}

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

  inline reinforcement_learning::action_prob operator*() const
  {
    return *this->current;
  }
};

API reinforcement_learning::slot_ranking* CreateSlotRanking()
{
  return new reinforcement_learning::slot_ranking();
}

API void DeleteSlotRanking(reinforcement_learning::slot_ranking* slot)
{
  delete slot;
}

API const char* GetSlotId(reinforcement_learning::slot_ranking* slot, int& slot_id_size)
{
  const auto slot_id = slot->get_id();
  slot_id_size = static_cast<int>(slot_id.size());
  return slot_id.data();
}

API size_t GetSlotActionCount(reinforcement_learning::slot_ranking* slot)
{
  return slot->size();
}

API int GetSlotChosenAction(reinforcement_learning::slot_ranking* slot, size_t* action_id, reinforcement_learning::api_status* status)
{
  return slot->get_chosen_action_id(*action_id, status);
}

API slot_enumerator_adapter* CreateSlotEnumeratorAdapter(reinforcement_learning::slot_ranking* slot)
{
  return new slot_enumerator_adapter(slot);
}

API void DeleteSlotEnumeratorAdapter(slot_enumerator_adapter* adapter)
{
  delete adapter;
}

API int SlotEnumeratorInit(slot_enumerator_adapter* adapter)
{
  return adapter->check_current();
}

API int SlotEnumeratorMoveNext(slot_enumerator_adapter* adapter)
{
  return adapter->move_next();
}

API reinforcement_learning::action_prob_d GetSlotEnumeratorCurrent(slot_enumerator_adapter* adapter)
{
  return **adapter;
}
