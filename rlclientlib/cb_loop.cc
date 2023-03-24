#include "cb_loop.h"

#include "live_model_impl.h"

namespace reinforcement_learning
{
int cb_loop::choose_rank(const char* event_id, string_view context_json, ranking_response& response, api_status* status)
{
  INIT_CHECK();
  return choose_rank(event_id, context_json, action_flags::DEFAULT, response, status);
}

int cb_loop::choose_rank(string_view context_json, ranking_response& response, api_status* status)
{
  INIT_CHECK();
  return choose_rank(context_json, action_flags::DEFAULT, response, status);
}

int cb_loop::choose_rank(
    const char* event_id, string_view context_json, unsigned int flags, ranking_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->choose_rank(event_id, context_json, flags, response, status);
}

int cb_loop::choose_rank(string_view context_json, unsigned int flags, ranking_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->choose_rank(context_json, flags, response, status);
}

int cb_loop::report_outcome(const char* event_id, const char* outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id, outcome, status);
}

int cb_loop::report_outcome(const char* event_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id, outcome, status);
}

}  // namespace reinforcement_learning
