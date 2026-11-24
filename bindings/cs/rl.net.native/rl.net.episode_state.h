#pragma once

#include "rl.net.native.h"

#include <api_status.h>
#include <ranking_response.h>

// Global exports
extern "C"
{
  // NOTE: THIS IS NOT POLYMORPHISM SAFE!
  API reinforcement_learning::episode_state* CreateEpisodeState(const char* episodeId);
  API void DeleteEpisodeState(reinforcement_learning::episode_state* episode_state);

  API const char* GetEpisodeId(reinforcement_learning::episode_state* episode_state);
  API int UpdateEpisodeHistory(reinforcement_learning::episode_state* episode_state, const char* event_id,
      const char* previous_event_id, const char* context, const reinforcement_learning::ranking_response& resp,
      reinforcement_learning::api_status* error = nullptr);
}