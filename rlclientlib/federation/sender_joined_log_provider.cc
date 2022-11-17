

#include "sender_joined_log_provider.h"

#include "api_status.h"
#include "constants.h"
#include "err_constants.h"
#include "federation/eud_utils.h"
#include "future_compat.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/FileFormat_generated.h"
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
#include <flatbuffers/flatbuffer_builder.h>

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <ratio>
#include <string>
#include <vector>

using namespace reinforcement_learning;

namespace
{
class buffer_reader : public VW::io::reader
{
public:
  buffer_reader(std::vector<uint8_t>&& buffer)
      : VW::io::reader(true), _buffer(std::move(buffer)), _read_head(_buffer.data())
  {
  }
  ~buffer_reader() override = default;
  ssize_t read(char* buffer, size_t num_bytes) override
  {
    num_bytes = std::min((_buffer.data() + _buffer.size()) - _read_head, static_cast<std::ptrdiff_t>(num_bytes));
    if (num_bytes == 0) { return 0; }

    std::memcpy(buffer, _read_head, num_bytes);
    _read_head += num_bytes;

    return num_bytes;
  }
  void reset() override { _read_head = _buffer.data(); }

private:
  std::vector<uint8_t> _buffer;
  uint8_t* _read_head;
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

reinforcement_learning::messages::flatbuff::v2::TimeStamp from_rl_timestamp(const timestamp& ts)
{
  return reinforcement_learning::messages::flatbuff::v2::TimeStamp(
      ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.sub_second);
}

int emit_filemagic_message(std::vector<uint8_t>& output)
{
  // TODO consider doing this a safer way
  // Check endianness requirement of format and make sure we get that right here.
  uint32_t filemagic = 0x42465756;
  output.reserve(output.size() + 4);
  output.insert(std::end(output), reinterpret_cast<uint8_t*>(&filemagic), reinterpret_cast<uint8_t*>(&filemagic) + 4);
  return 0;
}

int emit_regular_message(std::vector<uint8_t>& output, flatbuffers::FlatBufferBuilder& fbb,
    flatbuffers::Offset<reinforcement_learning::messages::flatbuff::v2::JoinedPayload> joined_payload)
{
  // TODO consider doing this a safer way
  // Check endianness requirement of format and make sure we get that right here.
  uint32_t regular = 0xFFFFFFFF;
  output.reserve(output.size() + 4);
  output.insert(std::end(output), reinterpret_cast<uint8_t*>(&regular), reinterpret_cast<uint8_t*>(&regular) + 4);

  fbb.Finish(joined_payload);
  auto buffer = fbb.Release();

  uint32_t size = buffer.size();
  output.reserve(output.size() + 4);
  output.insert(std::end(output), reinterpret_cast<uint8_t*>(&size), reinterpret_cast<uint8_t*>(&size) + 4);

  output.reserve(output.size() + size);
  output.insert(std::end(output), buffer.data(), buffer.data() + size);
  return 0;
}
}  // namespace

namespace reinforcement_learning
{
RL_ATTR(nodiscard)
int sender_joined_log_provider::create(std::unique_ptr<sender_joined_log_provider>& output,
    const utility::configuration& config, i_trace* trace_logger, api_status* status)
{
  if (config.get_int(reinforcement_learning::name::PROTOCOL_VERSION, 999) != 2)
  { RETURN_ERROR_LS(trace_logger, status, invalid_argument) << " protocol version 2 required"; }

  const auto* eud_duration = config.get(reinforcement_learning::name::EUD_DURATION, "UNSET");
  if (eud_duration == reinforcement_learning::string_view("UNSET"))
  { RETURN_ERROR_ARG(trace_logger, status, invalid_argument, "eudduration must be set"); }

  std::chrono::seconds eud_offset;
  RETURN_IF_FAIL(reinforcement_learning::parse_eud(eud_duration, eud_offset, status));

  output = std::unique_ptr<sender_joined_log_provider>(new sender_joined_log_provider(eud_offset, trace_logger));
  return error_code::success;
}

sender_joined_log_provider::sender_joined_log_provider(std::chrono::seconds eud_offset, i_trace* trace_logger)
    : _eud_offset(eud_offset), _trace_logger(trace_logger)
{
}

RL_ATTR(nodiscard)
int sender_joined_log_provider::invoke_join(std::unique_ptr<VW::io::reader>& batch, api_status* status)
{
  std::lock_guard<std::mutex> lock(_mutex);
  std::vector<uint8_t> result;
  const auto eud_cutoff = std::chrono::system_clock::now() - _eud_offset;
  emit_filemagic_message(result);
  for (auto it = _interactions.cbegin(); it != _interactions.cend();)
  {
    flatbuffers::FlatBufferBuilder fbb;
    std::vector<flatbuffers::Offset<reinforcement_learning::messages::flatbuff::v2::JoinedEvent>> events;

    const auto& item = *it;
    const auto interaction_time = chrono_from_timestamp(std::get<0>(it->first));
    if (interaction_time <= eud_cutoff)
    {
      const auto reward_cutoff = interaction_time + _eud_offset;

      auto ts = from_rl_timestamp(std::get<0>(it->first));
      events.push_back(reinforcement_learning::messages::flatbuff::v2::CreateJoinedEventDirect(fbb, &it->second, &ts));
      const auto event_id = std::get<1>(it->first);
      const auto& observations = _observations.find(event_id);
      if (observations != _observations.end())
      {
        for (const auto& observation : observations->second)
        {
          auto observation_time = chrono_from_timestamp(std::get<0>(observation));
          if (observation_time <= reward_cutoff)
          {
            auto ts = from_rl_timestamp(std::get<0>(observation));
            events.push_back(reinforcement_learning::messages::flatbuff::v2::CreateJoinedEventDirect(
                fbb, &std::get<1>(observation), &ts));
          }
        }
        _observations.erase(event_id);
      }

      auto joined_payload = reinforcement_learning::messages::flatbuff::v2::CreateJoinedPayloadDirect(fbb, &events);
      emit_regular_message(result, fbb, joined_payload);
      _interactions.erase(it++);
    }
    else
    {
      // TODO make sure this is correct. As in does the timestamp ordering mean that all further events are within eud.
      // break;
      ++it;
    }
  }

  batch.reset(new buffer_reader(std::move(result)));

  return 0;
}

int sender_joined_log_provider::receive_events(const i_sender::buffer& data, reinforcement_learning::api_status* status)
{
  std::lock_guard<std::mutex> lock(_mutex);

  reinforcement_learning::logger::preamble pre;
  pre.read_from_bytes(data->preamble_begin(), reinforcement_learning::logger::preamble::size());

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

}  // namespace reinforcement_learning
