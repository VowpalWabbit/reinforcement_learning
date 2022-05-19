#pragma once

#include <cstddef>
#include <stdint.h>

#include <utility>
#include <vector>
#include <string>
#include <map>
#include <memory>

#include "ranking_response.h"
#include "rl_string_view.h"
#include "vw/common/vwvis.h"

namespace reinforcement_learning {
  class episode_history {
  public:
    episode_history() = default;
    VW_DLL_PUBLIC episode_history(const episode_history* previous);

    episode_history(const episode_history& other) = default;
    episode_history& operator=(const episode_history& other) = default;

    episode_history(episode_history&& other) = default;
    episode_history& operator=(episode_history&& other) = default;

    VW_DLL_PUBLIC void update(const char* event_id, const char* previous_event_id, string_view context, const ranking_response& resp);
    VW_DLL_PUBLIC std::string get_context(const char* previous_event_id, string_view context) const;

    VW_DLL_PUBLIC size_t size() const;

  private:
    int get_depth(const char* id) const;

  private:
    std::map<std::string, int> _depths;
  };

  class episode_state {
  public:
    VW_DLL_PUBLIC explicit episode_state(const char* episode_id);

    episode_state(const episode_state& other) = default;
    episode_state& operator=(const episode_state& other) = default;

    episode_state(episode_state&& other) = default;
    episode_state& operator=(episode_state&& other) = default;

    VW_DLL_PUBLIC const char* get_episode_id() const;
    VW_DLL_PUBLIC const episode_history& get_history() const;
    VW_DLL_PUBLIC size_t size() const;

    VW_DLL_PUBLIC int update(const char* event_id, const char* previous_event_id, string_view context, const ranking_response& response, api_status* error = nullptr);

  private:
    std::string _episode_id;

    episode_history _history;
  };

}
