#pragma once

#include "api_status.h"
#include "configuration.h"
#include "federation/event_sink.h"
#include "federation/joined_log_provider.h"
#include "future_compat.h"
#include "sender.h"
#include "time_helper.h"
#include "vw/io/io_adapter.h"

#include <map>
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
  virtual int receive_events(const i_sender::buffer& data, api_status* status) override;

  virtual ~sender_joined_log_provider() = default;

private:
  sender_joined_log_provider(std::chrono::seconds eud_offset, i_trace* trace_logger);
  std::map<std::tuple<timestamp, std::string>, std::vector<uint8_t>> _interactions;
  std::unordered_map<std::string, std::vector<std::tuple<timestamp, std::vector<uint8_t>>>> _observations;
  std::chrono::seconds _eud_offset;
  std::mutex _mutex;
  reinforcement_learning::i_trace* _trace_logger = nullptr;
};

}  // namespace reinforcement_learning
