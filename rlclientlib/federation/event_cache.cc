#include "event_cache.h"

namespace reinforcement_learning
{
int event_cache_memory::v_send(const buffer& data, api_status* status)
{
  _events.push_back(data);
  return error_code::success;
}

std::vector<std::shared_ptr<utility::data_buffer>> event_cache_memory::get_events()
{
  std::vector<buffer> events;
  std::swap(_events, events);
  return events;
}

}  // namespace reinforcement_learning
