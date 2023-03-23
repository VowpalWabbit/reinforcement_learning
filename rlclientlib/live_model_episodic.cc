#include "live_model_episodic.h"
#include "live_model_impl.h"

namespace reinforcement_learning
{
int live_model_episodic::report_outcome(
    const char* primary_id, int secondary_id, const char* outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(primary_id, secondary_id, outcome, status);
}

int live_model_episodic::report_outcome(const char* primary_id, int secondary_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(primary_id, secondary_id, outcome, status);
}

int live_model_episodic::report_outcome(
    const char* primary_id, const char* secondary_id, const char* outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(primary_id, secondary_id, outcome, status);
}

int live_model_episodic::report_outcome(
    const char* primary_id, const char* secondary_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(primary_id, secondary_id, outcome, status);
}

int live_model_episodic::request_episodic_decision(const char* event_id, const char* previous_id,
    string_view context_json, ranking_response& resp, episode_state& episode, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_episodic_decision(
      event_id, previous_id, context_json, action_flags::DEFAULT, resp, episode, status);
}

int live_model_episodic::request_episodic_decision(const char* event_id, const char* previous_id,
    string_view context_json, unsigned int flags, ranking_response& resp, episode_state& episode, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_episodic_decision(event_id, previous_id, context_json, flags, resp, episode, status);
}

}  // namespace reinforcement_learning
