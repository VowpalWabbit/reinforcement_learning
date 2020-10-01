#include "multistep.h"
#include "err_constants.h"

namespace reinforcement_learning {
  episode_history::episode_history(const episode_history* previous)
  : _previous(previous)
  , _depth(_previous == nullptr ? 1 : _previous->get_depth() + 1)
  {}

  int episode_history::get_depth() const {
    return _depth;
  }

  episode_state::episode_state(const char* episode_id)
  : _episode_id(episode_id) {}

  const char* episode_state::get_episode_id() const {
    return _episode_id.c_str();
  }

  const episode_history* episode_state::get_history(const char* previous_event_id) const {
    if (previous_event_id == nullptr) return nullptr;
    auto result = _history.find(previous_event_id);
    return result == _history.end() ? nullptr : result->second.get();
  }

  int episode_state::update(const char* previous_event_id, const char* context, const ranking_response& response, api_status* status) {
    _history[response.get_event_id()].reset(new episode_history(previous_event_id == nullptr ? nullptr : _history[previous_event_id].get()));
    return error_code::success;
  }
}