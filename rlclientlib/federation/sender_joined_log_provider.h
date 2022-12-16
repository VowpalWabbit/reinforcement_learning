#pragma once

#include "api_status.h"
#include "configuration.h"
#include "federation/event_sink.h"
#include "federation/joined_log_provider.h"
#include "future_compat.h"
#include "sender.h"
#include "time_helper.h"
#include "vw/io/io_adapter.h"

//#include <map>
#include <mutex>
#include <set>
#include <tuple>
#include <unordered_map>

namespace reinforcement_learning
{
class sender_joined_log_provider : public i_joined_log_provider, public i_event_sink
{
public:
  RL_ATTR(nodiscard)
  static int create(std::unique_ptr<sender_joined_log_provider>& output, const utility::configuration& config,
      i_trace* trace_logger = nullptr, api_status* status = nullptr);

  // Perform EUD joining on events that have previously been added
  // Output is a binary joined log that can be consumed by the binary parser
  RL_ATTR(nodiscard)
  virtual int invoke_join(std::unique_ptr<VW::io::reader>& batch, api_status* status = nullptr) override;

  // Add an event batch to the joiner
  // Input should consist of a preamble and an EventBatch flatbuffer
  RL_ATTR(nodiscard)
  virtual int receive_events(const i_sender::buffer& data, api_status* status = nullptr) override;

  virtual ~sender_joined_log_provider() = default;

private:
  // Internal object to store event data
  struct event_data
  {
    const std::string _event_id;
    const uint8_t* _data_ptr;
    const size_t _size;
    const timestamp _time;

    // We must hold a copy of i_sender::buffer so that its shared_ptr doesn't go out of scope
    i_sender::buffer _data_buffer;

    event_data(std::string event_id, const uint8_t* data_ptr, size_t size, timestamp time, i_sender::buffer data_buffer)
        : _event_id(std::move(event_id))
        , _data_ptr(data_ptr)
        , _size(size)
        , _time(time)
        , _data_buffer(std::move(data_buffer))
    {
    }

    // Sort events by time, then by event_id
    bool operator<(const event_data& other) const
    {
      return std::tie(_time, _event_id) < std::tie(other._time, other._event_id);
    }
  };

  sender_joined_log_provider(std::chrono::seconds eud_offset, i_trace* trace_logger);

  // Set of interaction events, sorted by time
  std::set<event_data> _interactions;

  // Map from event_id to vector of event_data objects
  std::unordered_map<std::string, std::vector<event_data>> _observations;

  std::chrono::seconds _eud_offset;
  reinforcement_learning::i_trace* _trace_logger = nullptr;
  std::mutex _mutex;
};

}  // namespace reinforcement_learning
