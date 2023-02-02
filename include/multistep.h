#pragma once

#include "ranking_response.h"
#include "rl_string_view.h"

#include <stdint.h>

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace reinforcement_learning
{
class episode_history
{
public:
  episode_history() = default;
  episode_history(const episode_history* previous);

  episode_history(const episode_history& other) = default;
  episode_history& operator=(const episode_history& other) = default;

  episode_history(episode_history&& other) = default;
  episode_history& operator=(episode_history&& other) = default;

  void update(const char* event_id, const char* previous_event_id, string_view context, const ranking_response& resp);
  std::string get_context(const char* previous_event_id, string_view context) const;

  size_t size() const;

private:
  int get_depth(const char* id) const;

private:
  std::map<std::string, int> _depths;
};

class episode_state
{
public:
  explicit episode_state(const char* episode_id);

  episode_state(const episode_state& other) = default;
  episode_state& operator=(const episode_state& other) = default;

  episode_state(episode_state&& other) = default;
  episode_state& operator=(episode_state&& other) = default;

  const char* get_episode_id() const;
  const episode_history& get_history() const;
  size_t size() const;

  int update(const char* event_id, const char* previous_event_id, string_view context, const ranking_response& response,
      api_status* error = nullptr);

private:
  std::string _episode_id;

  episode_history _history;
};

}  // namespace reinforcement_learning
