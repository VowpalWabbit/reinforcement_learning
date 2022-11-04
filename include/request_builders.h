#include "action_flags.h"
#include "continuous_action_response.h"
#include "decision_response.h"
#include "multi_slot_response.h"
#include "multi_slot_response_detailed.h"
#include "ranking_response.h"
#include "rl_string_view.h"

namespace reinforcement_learning
{
class live_model_impl;

template <class FluentBuilderT>
class basic_builder
{
protected:
  const char* _event_id = nullptr;
  unsigned int _flags = action_flags::DEFAULT;
  string_view _context_json;

public:
  FluentBuilderT& set_event_id(const char* event_id)
  {
    _event_id = event_id;
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT& set_flags(unsigned int flags)
  {
    _flags = flags;
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT& set_context(string_view context_json)
  {
    _context_json = context_json;
    return *static_cast<FluentBuilderT*>(this);
  }

  FluentBuilderT& clear()
  {
    _event_id = nullptr;
    _flags = action_flags::DEFAULT;
    _context_json = {};
    return *static_cast<FluentBuilderT*>(this);
  }
};

class rank_builder : public basic_builder<rank_builder>
{
  live_model_impl* model;

public:
  rank_builder(live_model_impl* live_model);
  int rank(ranking_response& resp, api_status* status = nullptr);
  rank_builder& clear();
};
}  // namespace reinforcement_learning