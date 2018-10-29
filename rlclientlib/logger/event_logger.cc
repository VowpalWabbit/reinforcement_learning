#include "event_logger.h"
#include "ranking_event.h"
#include "err_constants.h"

namespace reinforcement_learning
{
  namespace u = utility;

  int interaction_logger::log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status) {
    u::pooled_object_guard<u::data_buffer, u::buffer_factory> guard(_buffer_pool, _buffer_pool.get_or_create());
    guard->reset();
    return append(std::move(ranking_event::choose_rank(*guard.get(), event_id, context, flags, response)), status);
  }

  int observation_logger::report_action_taken(const char* event_id, api_status* status) {
    u::pooled_object_guard<u::data_buffer, u::buffer_factory> guard(_buffer_pool, _buffer_pool.get_or_create());
    guard->reset();
    return append(std::move(outcome_event::report_action_taken(*guard.get(), event_id)), status);
  }
}