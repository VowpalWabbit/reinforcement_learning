#include "multistep.h"

#include "err_constants.h"

namespace reinforcement_learning
{
void episode_history::update(
    const char* event_id, const char* previous_event_id, string_view context)
{
  _depths[event_id] = this->get_depth(previous_event_id) + 1;
}

std::string episode_history::get_context(const char* previous_event_id, string_view context) const
{
  return R"({"episode":{"depth":")" + std::to_string(this->get_depth(previous_event_id) + 1) + "\"}," +
      std::string(context.data() + 1);
}

int episode_history::get_depth(const char* id) const
{
  if (id == nullptr) { return 0; }
  auto result = _depths.find(id);
  return (result == _depths.end()) ? 0 : result->second;
}

size_t episode_history::size() const { return _depths.size(); }

episode_state::episode_state(const char* episode_id) : _episode_id(episode_id) {}

const char* episode_state::get_episode_id() const { return _episode_id.c_str(); }

const episode_history& episode_state::get_history() const { return _history; }

size_t episode_state::size() const { return _history.size(); }

int episode_state::update(const char* event_id, const char* previous_event_id, string_view context, 
    api_status* status)
{
  _history.update(event_id, previous_event_id, context);
  return error_code::success;
}
}  // namespace reinforcement_learning
