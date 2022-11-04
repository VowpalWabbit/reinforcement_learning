#pragma once

#include "api_status.h"
#include "data_buffer.h"
#include "sender.h"

#include <vector>

namespace reinforcement_learning
{
class i_event_cache : public i_sender
{
  using buffer = std::shared_ptr<utility::data_buffer>;

public:
  virtual std::vector<buffer> get_events() = 0;
};

class event_cache_memory : public i_event_cache
{
  using buffer = std::shared_ptr<utility::data_buffer>;

public:
  event_cache_memory() = default;
  virtual int init(const utility::configuration& config, api_status* status) override { return error_code::success; }

  // add an event to the cache
  // this does not perform any parsing or validation on the event data
  virtual int v_send(const buffer& data, api_status* status = nullptr) override;

  // get cached events and clear the cache
  virtual std::vector<buffer> get_events() override;

private:
  std::vector<buffer> _events;
};

/*
// other event queue implementations
class event_cache_disk : public i_sender, public i_event_source {};
*/

}  // namespace reinforcement_learning
