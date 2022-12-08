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

#include <flatbuffers/flatbuffers.h>

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

timestamp to_rl_timestamp(const messages::flatbuff::v2::TimeStamp& ts)
{
  return timestamp(ts.year(), ts.month(), ts.day(), ts.hour(), ts.minute(), ts.second(), ts.subsecond());
}

messages::flatbuff::v2::TimeStamp from_rl_timestamp(const timestamp& ts)
{
  return messages::flatbuff::v2::TimeStamp(ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.sub_second);
}

void emit_uint32(std::vector<uint8_t>& output, uint32_t data)
{
  // TODO consider doing this a safer way
  // Check endianness requirement of format and make sure we get that right here.
  output.reserve(output.size() + sizeof(uint32_t));
  output.insert(
      std::end(output), reinterpret_cast<uint8_t*>(&data), reinterpret_cast<uint8_t*>(&data) + sizeof(uint32_t));
}

int emit_filemagic_message(std::vector<uint8_t>& output)
{
  constexpr uint32_t filemagic = 0x42465756;
  constexpr uint32_t version = 1;
  emit_uint32(output, filemagic);
  emit_uint32(output, version);
  return 0;
}

int emit_regular_message(std::vector<uint8_t>& output, flatbuffers::FlatBufferBuilder& fbb,
    flatbuffers::Offset<messages::flatbuff::v2::JoinedPayload> joined_payload)
{
  constexpr uint32_t regular = 0xFFFFFFFF;
  emit_uint32(output, regular);

  fbb.Finish(joined_payload);
  auto buffer = fbb.Release();

  uint32_t size = buffer.size();
  emit_uint32(output, size);

  output.reserve(output.size() + size);
  output.insert(std::end(output), buffer.data(), buffer.data() + size);

  uint32_t padding_size = size % 8;
  output.reserve(output.size() + padding_size);
  output.insert(std::end(output), padding_size, 0);

  return 0;
}
}  // namespace

namespace reinforcement_learning
{
RL_ATTR(nodiscard)
int sender_joined_log_provider::create(std::unique_ptr<sender_joined_log_provider>& output,
    const utility::configuration& config, i_trace* trace_logger, api_status* status)
{
  if (config.get_int(name::PROTOCOL_VERSION, 999) != 2)
  {
    RETURN_ERROR_LS(trace_logger, status, invalid_argument) << " protocol version 2 required";
  }

  std::string eud_duration = config.get(name::EUD_DURATION, "UNSET");
  if (eud_duration == "UNSET") { RETURN_ERROR_ARG(trace_logger, status, invalid_argument, "eudduration must be set"); }

  std::chrono::seconds eud_offset;
  RETURN_IF_FAIL(parse_eud(eud_duration, eud_offset, status));

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
    std::vector<flatbuffers::Offset<messages::flatbuff::v2::JoinedEvent>> events;

    const auto& item = *it;
    const auto interaction_timestamp = std::get<0>(it->first);
    const auto interaction_time = interaction_timestamp.to_time_point();
    if (interaction_time <= eud_cutoff)
    {
      const auto reward_cutoff = interaction_time + _eud_offset;

      auto ts = from_rl_timestamp(std::get<0>(it->first));
      events.push_back(messages::flatbuff::v2::CreateJoinedEventDirect(fbb, &it->second, &ts));
      const auto& event_id = std::get<1>(it->first);
      const auto& observations = _observations.find(event_id);
      if (observations != _observations.end())
      {
        for (const auto& observation : observations->second)
        {
          const auto observation_timestamp = std::get<0>(observation);
          const auto observation_time = observation_timestamp.to_time_point();
          if (observation_time <= reward_cutoff)
          {
            auto ts = from_rl_timestamp(std::get<0>(observation));
            events.push_back(messages::flatbuff::v2::CreateJoinedEventDirect(fbb, &std::get<1>(observation), &ts));
          }
        }
        _observations.erase(event_id);
      }

      auto joined_payload = messages::flatbuff::v2::CreateJoinedPayloadDirect(fbb, &events);
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

int sender_joined_log_provider::receive_events(const i_sender::buffer& data, api_status* status)
{
  std::lock_guard<std::mutex> lock(_mutex);

  logger::preamble pre;
  pre.read_from_bytes(data->preamble_begin(), logger::preamble::size());

  if (pre.msg_type != logger::message_type::fb_generic_event_collection)
  {
    RETURN_ERROR_LS(_trace_logger, status, invalid_argument)
        << " Message type " << pre.msg_type << " cannot be handled.";
  }

  auto res = messages::flatbuff::v2::GetEventBatch(data->body_begin());
  flatbuffers::Verifier verifier(data->body_begin(), data->body_filled_size());
  auto result = res->Verify(verifier);

  if (!result)
  {
    RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "verify failed for fb_generic_event_collection";
  }

  if (res->metadata()->content_encoding()->str() != "IDENTITY")
  {
    RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "Can only handle IDENTITY encoding";
  }

  for (auto serialized_event : *res->events())
  {
    const auto* serialized_payload = serialized_event->payload();
    const auto* event = flatbuffers::GetRoot<messages::flatbuff::v2::Event>(serialized_payload->data());

    std::string event_id = event->meta()->id()->str();
    std::vector<uint8_t> joined_event_data(
        serialized_payload->data(), serialized_payload->data() + serialized_payload->size());
    auto event_timestamp = to_rl_timestamp(*event->meta()->client_time_utc());

    switch (event->meta()->payload_type())
    {
      case messages::flatbuff::v2::PayloadType_CB:
      case messages::flatbuff::v2::PayloadType_CCB:
      case messages::flatbuff::v2::PayloadType_Slates:
      case messages::flatbuff::v2::PayloadType_CA:
      case messages::flatbuff::v2::PayloadType_MultiStep:
        _interactions.emplace(std::make_tuple(event_timestamp, event_id), std::move(joined_event_data));
        break;

      case messages::flatbuff::v2::PayloadType_Outcome:
        _observations[event_id].emplace_back(std::make_tuple(event_timestamp, std::move(joined_event_data)));
        break;

      default:
        RETURN_ERROR_LS(_trace_logger, status, invalid_argument)
            << "Could not process payload type: " << event->meta()->payload_type();
    }
  }
  return error_code::success;
}

}  // namespace reinforcement_learning
