#pragma once
#include "str_util.h"

#include <map>

template <typename T>
class simulation_stats
{
public:
  simulation_stats() = default;
  ~simulation_stats() = default;
  void record(const std::string& id, T chosen_action, const float outcome)
  {
    auto& action_stats = _action_stats[std::make_pair(id, chosen_action)];
    if (outcome > 0.00001f) ++action_stats.first;
    ++action_stats.second;
    auto& item_count = _item_stats[id];
    ++item_count;
    ++_total_events;
  }

  std::string get_stats(const std::string& id, T chosen_action)
  {
    auto& action_stats = _action_stats[std::make_pair(id, chosen_action)];
    auto& item_count = _item_stats[id];

    return u::concat("wins: ", action_stats.first, ", out_of: ", action_stats.second, ", total: ", item_count);
  }

  int count() const { return _total_events; }

private:
  std::map<std::pair<std::string, T>, std::pair<int, int>> _action_stats;
  std::map<std::string, int> _item_stats;
  int _total_events = 0;
};