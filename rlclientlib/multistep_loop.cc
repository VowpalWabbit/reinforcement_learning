#include "multistep_loop.h"

#include "live_model_impl.h"

namespace reinforcement_learning
{
int multistep_loop::request_episodic_decision(str_view event_id, str_view previous_id, str_view context_json,
      ranking_response& resp, episode_state& episode, api_status* status)
{
    INIT_CHECK();
    return request_episodic_decision(event_id, previous_id, context_json, action_flags::DEFAULT, resp, episode, status);
}

int multistep_loop::request_episodic_decision(str_view event_id, str_view previous_id, str_view context_json,
      unsigned int flags, ranking_response& resp, episode_state& episode, api_status* status)
{
    INIT_CHECK();
    return _pimpl->request_episodic_decision(event_id.str, previous_id.str, {context_json.str, context_json.size}, flags, resp, episode, status);
}

int multistep_loop::report_outcome(str_view episode_id, str_view event_id, float outcome, api_status* status)
{
    INIT_CHECK();
    return _pimpl->report_outcome(episode_id.str, event_id.str, outcome, status);
}

int multistep_loop::report_outcome(str_view episode_id, str_view event_id, str_view outcome, api_status* status)
{
    INIT_CHECK();
    return _pimpl->report_outcome(episode_id.str, event_id.str, outcome.str, status);
}

int multistep_loop::report_outcome(str_view event_id, episode_state& episode, float outcome, api_status* status)
{
    INIT_CHECK();
    return _pimpl->report_outcome(event_id.str, episode.get_episode_id(), outcome, status);
}

int multistep_loop::report_outcome(str_view event_id, episode_state& episode, str_view outcome, api_status* status)
{
    INIT_CHECK();
    return _pimpl->report_outcome(event_id.str, episode.get_episode_id(), outcome.str, status);
}

int multistep_loop::report_outcome(str_view episode_id, float outcome, api_status* status)
{
    INIT_CHECK();
    return _pimpl->report_outcome(episode_id.str, outcome, status);
}

int multistep_loop::report_outcome(str_view episode_id, str_view outcome, api_status* status)
{
    INIT_CHECK();
    return _pimpl->report_outcome(episode_id.str, outcome.str, status);
}

}