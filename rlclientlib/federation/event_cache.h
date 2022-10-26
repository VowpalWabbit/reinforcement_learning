#pragma once

#include "api_status.h"
#include "sender.h"

#include <vector>

namespace reinforcement_learning
{

class event_cache_memory : public i_sender
{
public:
  event_cache_memory() = default;

  // add an event to the cache
  // this does not perform any parsing or validation on the event data
  virtual int v_send(const buffer& data, api_status* status = nullptr) override;

  // get cached events and clear the cache
  std::vector<buffer> get_events();

private:
  std::vector<buffer> _events;
};

/*
// other event queue implementations
class event_cache_disk : public i_sender {};
*/

}
