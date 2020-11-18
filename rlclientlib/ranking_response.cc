#include "ranking_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning {

  char const* ranking_response::get_event_id() const {
    return get_join_id();
  }

  void ranking_response::set_event_id(char const* event_id) {
    set_join_id(event_id);
  }
}
