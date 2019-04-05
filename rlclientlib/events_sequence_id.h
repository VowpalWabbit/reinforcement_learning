#pragma once
#include "ranking_event.h"

namespace reinforcement_learning {
  class events_sequence_id {
  public:
    events_sequence_id() = default;
    events_sequence_id(const event& first, const event& second, size_t count);
    
    void set_first(const event& first);
    void set_last(const event& last);
    void set_count(size_t count);

    std::string get() const;
  private:
    std::string _first;
    std::string _last;
    size_t _count;
  };
}
