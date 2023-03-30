#include "cb_loop.h"

#include "live_model_impl.h"

namespace reinforcement_learning
{
int cb_loop::choose_rank(loop_str event_id, loop_str context_json, ranking_response& response, api_status* status)
{
  INIT_CHECK();
  return choose_rank(event_id, context_json, action_flags::DEFAULT, response, status);
}

int cb_loop::choose_rank(loop_str context_json, ranking_response& response, api_status* status)
{
  INIT_CHECK();
  return choose_rank(context_json, action_flags::DEFAULT, response, status);
}

int cb_loop::choose_rank(
    loop_str event_id, loop_str context_json, unsigned int flags, ranking_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->choose_rank(event_id.str, string_view(context_json.str, context_json.size), flags, response, status);
}

int cb_loop::choose_rank(loop_str context_json, unsigned int flags, ranking_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->choose_rank(string_view(context_json.str, context_json.size), flags, response, status);
}

int cb_loop::report_outcome(loop_str event_id, loop_str outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id.str, outcome.str, status);
}

int cb_loop::report_outcome(loop_str event_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id.str, outcome, status);
}

}  // namespace reinforcement_learning
