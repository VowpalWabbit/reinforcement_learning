#include "events_sequence_id.h"
#include "str_util.h"

namespace reinforcement_learning {
  events_sequence_id::events_sequence_id(const event& first, const event& last, size_t count)
    : _first(first.get_event_id())
    , _last(last.get_event_id())
    , _count(count)
  {}

  std::string events_sequence_id::get() const {
    return utility::concat(_first.c_str(), "/", _last.c_str(), "/", _count);
  }

  void events_sequence_id::set_first(const event& first) {
    _first = first.get_event_id();
  }

  void events_sequence_id::set_last(const event& last) {
    _last = last.get_event_id();
  }

  void events_sequence_id::set_count(size_t count) {
    _count = count;
  }
}