#pragma once

#include "api_status.h"
#include "configuration.h"
#include "federation/joined_log_provider.h"
#include "future_compat.h"
#include "sender.h"
#include "time_helper.h"
#include "vw/io/io_adapter.h"

#include <map>
#include <unordered_map>

namespace reinforcement_learning
{
class sender_joined_log_provider : public i_joined_log_provider
{
public:
  int init(const reinforcement_learning::utility::configuration& config, reinforcement_learning::api_status* status);

  // Perform EUD joining on events that have previously been added
  // Output is a binary joined log that can be consumed by the binary parser
  RL_ATTR(nodiscard)
  int invoke_join(std::unique_ptr<VW::io::reader>& batch, api_status* status = nullptr) override;

  // Add an event batch to the joiner
  // Input should consist of a preamble and an EventBatch flatbuffer
  RL_ATTR(nodiscard)
  int add_events(const i_sender::buffer& data, reinforcement_learning::api_status* status);

  // Return an object of type i_sender that will forward data to add_events() of this object
  // Each call returns a new output, and the caller of this function takes ownership of it
  std::unique_ptr<i_sender> get_sender_proxy();

private:
  reinforcement_learning::i_trace* _trace_logger = nullptr;
  std::map<std::tuple<timestamp, std::string>, std::vector<uint8_t>> _interactions;
  std::unordered_map<std::string, std::vector<std::tuple<timestamp, std::vector<uint8_t>>>> _observations;
  std::chrono::seconds _eud_offset;
  std::mutex _mutex;
};

}  // namespace reinforcement_learning
