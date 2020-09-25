#include "multistep.h"
#include "err_constants.h"

namespace reinforcement_learning {
  const char* episode_history::get() const {
    return body.c_str();
  }

  int episode_history::update(const char* context, const ranking_response& response, api_status* status) {
    return error_code::success;//empty for now;
  }

  episode_state::episode_state(const char* episode_id)
  : _episode_id(episode_id) {}

  const char* episode_state::get_episode_id() const {
    return _episode_id.c_str();
  }

  const episode_history& episode_state::get_history() const {
    return _history;
  }

  int episode_state::update(const char* previous_event_id, const char* context, const ranking_response& response, api_status* status) {
    _history.update(context, response);
    return error_code::success;
  }
}