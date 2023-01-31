#include "rl.net.episode_state.h"

API reinforcement_learning::episode_state* CreateEpisodeState(const char* episodeId)
{
  return new reinforcement_learning::episode_state(episodeId);
}

API void DeleteEpisodeState(reinforcement_learning::episode_state* episode_state) { delete episode_state; }

API const char* GetEpisodeId(reinforcement_learning::episode_state* episode_state)
{
  return episode_state->get_episode_id();
}

API int UpdateEpisodeHistory(reinforcement_learning::episode_state* episode_state, const char* event_id,
    const char* previous_event_id, const char* context, reinforcement_learning::api_status* error)
{
  return episode_state->update(event_id, previous_event_id, context, error);
}