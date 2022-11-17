#pragma once

#include "api_status.h"
#include "data_buffer.h"
#include "future_compat.h"
#include "sender.h"

#include <vector>

namespace reinforcement_learning
{
class i_event_sink
{
  using buffer = std::shared_ptr<utility::data_buffer>;

public:
  // Add an event batch
  // Input should consist of a preamble and an EventBatch flatbuffer
  RL_ATTR(nodiscard)
  virtual int receive_events(const buffer& data, api_status* status = nullptr) = 0;

  // Return an object of type i_sender that will forward data to receive_events() of this object
  // Each call returns a new output, and the caller of this function takes ownership of it
  std::unique_ptr<i_sender> get_sender_proxy() { return std::unique_ptr<i_sender>(new sender_proxy(this)); }

  virtual ~i_event_sink() = default;

private:
  struct sender_proxy : public i_sender
  {
    sender_proxy(i_event_sink* event_sink) : _event_sink(event_sink) {}
    virtual int v_send(const buffer& data, api_status* status = nullptr) override
    {
      return _event_sink->receive_events(data, status);
    }
    virtual int init(const utility::configuration&, api_status*) override { return error_code::success; }
    virtual ~sender_proxy() = default;
    i_event_sink* _event_sink;
  };
};

}  // namespace reinforcement_learning
