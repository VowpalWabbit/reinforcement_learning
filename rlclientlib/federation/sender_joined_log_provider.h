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
struct sender_joined_log_provider : public i_joined_log_provider
{
  int init(const reinforcement_learning::utility::configuration& config, reinforcement_learning::api_status* status);

  RL_ATTR(nodiscard)
  int invoke_join(std::unique_ptr<VW::io::reader>& batch, api_status* status = nullptr) override;

  int v_send(const i_sender::buffer& data, reinforcement_learning::api_status* status);

private:
  reinforcement_learning::i_trace* _trace_logger = nullptr;
  std::map<std::tuple<timestamp, std::string>, std::vector<uint8_t>> _interactions;
  std::unordered_map<std::string, std::vector<std::tuple<timestamp, std::vector<uint8_t>>>> _observations;
  std::chrono::seconds _eud_offset;
  std::mutex _mutex;
};

struct sender_joined_log_provider_proxy : public reinforcement_learning::i_sender
{
  explicit sender_joined_log_provider_proxy(sender_joined_log_provider_proxy* sender) : _sender(sender) {}
  ~sender_joined_log_provider_proxy() override = default;

  int init(
      const reinforcement_learning::utility::configuration& config, reinforcement_learning::api_status* status) override
  {
    return error_code::success;
  }

protected:
  int v_send(const buffer& data, reinforcement_learning::api_status* status) override
  {
    return _sender->v_send(data, status);
  }

private:
  sender_joined_log_provider_proxy* _sender;
};
}  // namespace reinforcement_learning
