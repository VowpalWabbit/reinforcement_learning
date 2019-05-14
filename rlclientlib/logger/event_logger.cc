#include "event_logger.h"
#include "ranking_event.h"
#include "err_constants.h"

namespace reinforcement_learning { namespace logger {
  namespace u = utility;

  int interaction_logger::log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status) {
    return append(std::move(ranking_event::choose_rank(event_id, context, flags, response)), status);
  }

  int ccb_logger::log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const ranking_responses& response, api_status* status) {
    return append(std::move(decision_ranking_event::request_decision(event_ids, context, flags, response)), status);
  }

  int observation_logger::report_action_taken(const char* event_id, api_status* status) {
    return append(std::move(outcome_event::report_action_taken(event_id)), status);
}
}}
