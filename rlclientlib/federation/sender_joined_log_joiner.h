#pragma once

#include "api_status.h"
#include "constants.h"
#include "err_constants.h"
#include "federation/eud_utils.h"
#include "future_compat.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "joined_log_provider.h"
#include "logger/message_type.h"
#include "logger/preamble.h"
#include "rl_string_view.h"
#include "sender.h"
#include "time_helper.h"
#include "vw/common/text_utils.h"
#include "vw/io/io_adapter.h"

#include <flatbuffers/buffer.h>
#include <flatbuffers/detached_buffer.h>

#include <chrono>
#include <cstdint>
#include <map>
#include <ratio>
#include <string>
#include <vector>

namespace reinforcement_learning
{
struct binary_joined_log_batch : public i_joined_log_batch
{
  RL_ATTR(nodiscard)
  int next(std::unique_ptr<VW::io::reader>& chunk_reader, api_status* status = nullptr) override {}
};

timestamp to_rl_timestamp(const reinforcement_learning::messages::flatbuff::v2::TimeStamp& ts)
{
  timestamp output;
  output.year = ts.year();
  output.day = ts.day();
  output.month = ts.month();
  output.hour = ts.hour();
  output.minute = ts.minute();
  output.second = ts.second();
  output.sub_second = ts.subsecond();
  return output;
}

struct sender_joined_log_provider : public i_joined_log_provider, i_sender
{
  int init(
      const reinforcement_learning::utility::configuration& config, reinforcement_learning::api_status* status) override
  {
    if (config.get_int(reinforcement_learning::name::PROTOCOL_VERSION, 999) != 2)
    { RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << " protocol version 2 required"; }

    const auto* eud_duration = config.get(reinforcement_learning::name::EUD_DURATION, "UNSET");
    if (eud_duration == reinforcement_learning::string_view("UNSET"))
    { RETURN_ERROR_ARG(_trace_logger, status, invalid_argument, "eudduration must be set"); }
    RETURN_IF_FAIL(reinforcement_learning::parse_eud(eud_duration, _eud_offset, status));
    return error_code::success;
  }

  RL_ATTR(nodiscard)
  int invoke_join(std::unique_ptr<i_joined_log_batch>& batch, api_status* status = nullptr) override {
    const auto eud_cutoff = std::chrono::system_clock::now() - _eud_offset;
    for (auto it = _interactions.cbegin(); it != _interactions.cend();)
    {
      const auto& item = *it;
      const auto interaction_time = chrono_from_timestamp(std::get<0>(it->first));
      if (interaction_time <= eud_cutoff)
      {
        const auto reward_cutoff = interaction_time + _eud_offset;
        // TODO write interaction to flatbuffer buffer
        const auto event_id = std::get<1>(it->first);
        const auto& observations = _observations.find(event_id);
        if (observations != _observations.end())
        {
          for (const auto& observation : observations->second)
          {
            auto observation_time = chrono_from_timestamp(std::get<0>(observation));
            if (observation_time <= reward_cutoff)
            {
              // TODO write observation to flatbuffer buffer
            }
          }
          _observations.erase(event_id);
        }
        _interactions.erase(it++);
      }
      else
      {
        // TODO check that this works
        // break;
        ++it;
      }
    }
  }

protected:
  int v_send(const buffer& data, reinforcement_learning::api_status* status) override
  {
    std::lock_guard<std::mutex> lock(_mutex);

    reinforcement_learning::logger::preamble pre;
    pre.read_from_bytes(data->preamble_begin(), pre.size());

    if (pre.msg_type != reinforcement_learning::logger::message_type::fb_generic_event_collection)
    {
      RETURN_ERROR_LS(_trace_logger, status, invalid_argument)
          << " Message type " << pre.msg_type << " cannot be handled.";
    }

    auto res = reinforcement_learning::messages::flatbuff::v2::GetEventBatch(data->body_begin());
    flatbuffers::Verifier verifier(data->body_begin(), data->body_filled_size());
    auto result = res->Verify(verifier);

    if (!result)
    { RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "verify failed for fb_generic_event_collection"; }

    if (res->metadata()->content_encoding()->str() != "IDENTITY")
    { RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "Can only handle IDENTITY encoding"; }

    for (auto message : *res->events())
    {
      const auto* event_payload = message->payload();
      auto* event = flatbuffers::GetRoot<reinforcement_learning::messages::flatbuff::v2::Event>(event_payload);

      std::string event_id = event->meta()->id()->str();
      std::vector<uint8_t> payload(message->payload()->data(), message->payload()->data() + message->payload()->size());
      auto event_timestamp = to_rl_timestamp(*event->meta()->client_time_utc());

      if (event->meta()->payload_type() == reinforcement_learning::messages::flatbuff::v2::PayloadType_CB)
      { _interactions.emplace(std::make_tuple(event_timestamp, event_id), std::move(payload)); }

      if (event->meta()->payload_type() == reinforcement_learning::messages::flatbuff::v2::PayloadType_Outcome)
      { _observations[event_id].emplace_back(std::make_tuple(event_timestamp, std::move(payload))); }
    }
    return reinforcement_learning::error_code::success;
  }

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
