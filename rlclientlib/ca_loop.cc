#include "ca_loop.h"

#include "live_model_impl.h"

namespace reinforcement_learning
{
int ca_loop::request_continuous_action(str_view event_id, str_view context_json, unsigned int flags,
    continuous_action_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_continuous_action(
      event_id.str, string_view(context_json.str, context_json.size), flags, response, status);
}

int ca_loop::request_continuous_action(
    str_view event_id, str_view context_json, continuous_action_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_continuous_action(
      event_id.str, string_view(context_json.str, context_json.size), action_flags::DEFAULT, response, status);
}

int ca_loop::request_continuous_action(
    str_view context_json, unsigned int flags, continuous_action_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_continuous_action(string_view(context_json.str, context_json.size), flags, response, status);
}

int ca_loop::request_continuous_action(str_view context_json, continuous_action_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_continuous_action(
      string_view(context_json.str, context_json.size), action_flags::DEFAULT, response, status);
}

int ca_loop::report_outcome(str_view event_id, str_view outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id.str, outcome.str, status);
}

int ca_loop::report_outcome(str_view event_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id.str, outcome, status);
}
}  // namespace reinforcement_learning
