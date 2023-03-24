#include "ca_loop.h"

#include "live_model_impl.h"

namespace reinforcement_learning
{
int ca_loop::request_continuous_action(const char* event_id, string_view context_json, unsigned int flags,
    continuous_action_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_continuous_action(event_id, context_json, flags, response, status);
}

int ca_loop::request_continuous_action(
    const char* event_id, string_view context_json, continuous_action_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_continuous_action(event_id, context_json, action_flags::DEFAULT, response, status);
}

int ca_loop::request_continuous_action(
    string_view context_json, unsigned int flags, continuous_action_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_continuous_action(context_json, flags, response, status);
}

int ca_loop::request_continuous_action(
    string_view context_json, continuous_action_response& response, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_continuous_action(context_json, action_flags::DEFAULT, response, status);
}

int ca_loop::report_outcome(const char* event_id, const char* outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id, outcome, status);
}

int ca_loop::report_outcome(const char* event_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id, outcome, status);
}
}  // namespace reinforcement_learning
