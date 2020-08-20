#include "multistep.h"
#include "err_constants.h"

namespace reinforcement_learning {
  const char* episode_history::get() const {
    return body.c_str();
  }

  int episode_history::update(const char* context, const ranking_response& response, api_status* status) {
    return error_code::success;//empty for now;
  }

  const char* episode_state::get_episode_id() const {
    return episode_id.c_str();
  }

  const char* episode_state::get_last_event_id() const {
    return last_event_id.c_str();
  }

  const episode_history& episode_state::get_history() const {
    return history;
  }

  int episode_state::update(const char* context, const ranking_response& response, api_status* status) {
    last_event_id = response.get_event_id();
    history.update(context, response);
    return error_code::success;
  }
}