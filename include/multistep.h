#pragma once

#pragma once
#include <cstddef>
#include <stdint.h>

#include <utility>
#include <vector>
#include <string>

#include "ranking_response.h"

namespace reinforcement_learning {
  class episode_history {
  public:
    episode_history() = default;

    episode_history(const episode_history& other) = default;
    episode_history& operator=(const episode_history& other) = default;

    episode_history(episode_history&& other) = default;
    episode_history& operator=(episode_history&& other) = default;

    const char* get() const;
    int update(const char* context, const ranking_response& response, api_status* error = nullptr);

  private:
    std::string body;
  };

  class episode_state {
  public:
    episode_state(const char* episode_id);

    episode_state(const episode_state& other) = default;
    episode_state& operator=(const episode_state& other) = default;

    episode_state(episode_state&& other) = default;
    episode_state& operator=(episode_state&& other) = default;

    const char* get_episode_id() const;
    const episode_history& get_history() const;

    int update(const char* previous_event_id, const char* context, const ranking_response& response, api_status* error = nullptr);

  private:
    std::string _episode_id;
    episode_history _history;
  };

}
