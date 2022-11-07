#pragma once

#include "api_status.h"
#include "data_buffer.h"
#include "sender.h"

#include <vector>

namespace reinforcement_learning
{
class i_event_cache
{
  using buffer = std::shared_ptr<utility::data_buffer>;

public:
  // Get cached events and clear the cache
  virtual std::vector<buffer> get_events() = 0;

  // Add an event to the cache
  // This does not perform any parsing or validation on the event data
  virtual int add_event(const buffer& data, api_status* status = nullptr) = 0;

  // Return an object of type i_sender that will send data to the event cache
  // Each call returns a new output, and the caller of this function takes ownership of it
  std::unique_ptr<i_sender> get_sender_proxy() { return std::unique_ptr<i_sender>(new sender_proxy(this)); }

private:
  class sender_proxy : public i_sender
  {
  public:
    sender_proxy(i_event_cache* event_cache) : _event_cache(event_cache) {}
    virtual int v_send(const buffer& data, api_status* status = nullptr) override
    {
      return _event_cache->add_event(data, status);
    }
    virtual int init(const utility::configuration&, api_status*) override { return error_code::success; }
    virtual ~sender_proxy() = default;
    i_event_cache* _event_cache;
  };
};

class event_cache_memory : public i_event_cache
{
  using buffer = std::shared_ptr<utility::data_buffer>;

public:
  event_cache_memory() = default;

  virtual int add_event(const buffer& data, api_status* status = nullptr) override;
  virtual std::vector<buffer> get_events() override;

private:
  std::vector<buffer> _events;
};

/*
// other event queue implementations
class event_cache_disk : public i_sender, public i_event_source {};
*/

}  // namespace reinforcement_learning
