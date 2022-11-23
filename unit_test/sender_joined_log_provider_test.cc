#include <boost/test/unit_test.hpp>

#include "configuration.h"
#include "constants.h"
#include "data_buffer.h"
#include "err_constants.h"
#include "federation/sender_joined_log_provider.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "logger/message_type.h"
#include "logger/preamble.h"
#include "time_helper.h"
#include "vw/core/io_buf.h"
#include "vw/io/io_adapter.h"

#include <flatbuffers/flatbuffers.h>

#include <cstdint>
#include <set>
#include <vector>

using namespace reinforcement_learning;
namespace fbv2 = reinforcement_learning::messages::flatbuff::v2;

namespace
{
constexpr size_t BINARY_PARSER_VERSION = 1;
constexpr uint32_t MSG_TYPE_FILEMAGIC = 0x42465756;  //'VWFB'
constexpr uint32_t MSG_TYPE_HEADER = 0x55555555;
constexpr uint32_t MSG_TYPE_REGULAR = 0xFFFFFFFF;
constexpr uint32_t MSG_TYPE_CHECKPOINT = 0x11111111;
constexpr uint32_t MSG_TYPE_EOF = 0xAAAAAAAA;

// Object containing event metadata and string payload
// with helper functions to convert to/from Event flatbuffers
struct test_event
{
  std::string _event_id;
  timestamp _time;
  fbv2::PayloadType _payload_type;
  std::string _payload;

  test_event(std::string event_id, timestamp time, fbv2::PayloadType payload_type, std::string payload)
      : _event_id(std::move(event_id)), _time(time), _payload_type(payload_type), _payload(std::move(payload))
  {
  }

  test_event(const fbv2::Event* buffer)
  {
    auto meta = buffer->meta();
    auto ts = meta->client_time_utc();
    auto payload = buffer->payload();
    _event_id = flatbuffers::GetString(meta->id());
    _time.year = ts->year();
    _time.day = ts->day();
    _time.month = ts->month();
    _time.hour = ts->hour();
    _time.minute = ts->minute();
    _time.second = ts->second();
    _time.sub_second = ts->subsecond();
    _payload_type = meta->payload_type();
    _payload = std::string(payload->begin(), payload->end());
  }

  flatbuffers::DetachedBuffer to_flatbuffer()
  {
    flatbuffers::FlatBufferBuilder event_builder;
    auto ts =
        fbv2::TimeStamp(_time.year, _time.month, _time.day, _time.hour, _time.minute, _time.second, _time.sub_second);
    auto metadata = fbv2::CreateMetadataDirect(event_builder, _event_id.c_str(), &ts, nullptr, _payload_type);
    auto payload_bytes = std::vector<uint8_t>(_payload.begin(), _payload.end());
    auto event = fbv2::CreateEventDirect(event_builder, metadata, &payload_bytes);
    event_builder.Finish(event);
    return event_builder.Release();
  }
};  // struct test_event

inline bool operator<(const test_event& te1, const test_event& te2)
{
  return std::tie(te2._event_id, te2._time, te2._payload_type, te2._payload) <
      std::tie(te2._event_id, te2._time, te2._payload_type, te2._payload);
}

// A group of unique test_event objects with the same event id
struct test_event_batch
{
  std::set<test_event> _batch;

  bool add_event(test_event evt)
  {
    auto result = _batch.insert(std::move(evt));
    return std::get<1>(result);
  }

  bool verify()
  {
    // check that batch is non-empty and all events have the same id
    if (_batch.empty()) return false;
    auto first_id = _batch.begin()->_event_id;
    for (auto&& evt : _batch)
    {
      if (evt._event_id != first_id) return false;
    }
    return true;
  }
};  // struct test_event_batch

std::shared_ptr<utility::data_buffer> create_message(test_event t_event)
{
  // create the Event flatbuffer
  auto event_buffer = t_event.to_flatbuffer();
  auto serialized_buffer = std::vector<uint8_t>(event_buffer.data(), event_buffer.data() + event_buffer.size());

  // create the EventBatch flatbuffer
  flatbuffers::FlatBufferBuilder batch_builder;
  auto serialized_event = fbv2::CreateSerializedEventDirect(batch_builder, &serialized_buffer);
  std::vector<decltype(serialized_event)> serialized_event_vector = {serialized_event};
  auto batch_metadata = fbv2::CreateBatchMetadataDirect(batch_builder, "IDENTITY");
  auto event_batch = fbv2::CreateEventBatchDirect(batch_builder, &serialized_event_vector, batch_metadata);
  batch_builder.Finish(event_batch);
  auto event_batch_buffer = batch_builder.Release();

  // create preamble and put the flatbuffer into utility::data_buffer
  auto body_size = event_batch_buffer.size();
  logger::preamble pre = {.reserved = 0,
      .version = 0,
      .msg_type = logger::message_type::fb_generic_event_collection,
      .msg_size = static_cast<uint32_t>(body_size)};
  auto data_buffer = std::make_shared<utility::data_buffer>(body_size);
  BOOST_TEST(pre.write_to_bytes(data_buffer->preamble_begin(), data_buffer->preamble_size()));
  std::memcpy(data_buffer->body_begin(), event_batch_buffer.data(), event_batch_buffer.size());
  return data_buffer;
}

bool read_uint32(io_buf& buffer, uint32_t& output)
{
  char* read_ptr = nullptr;
  auto len = buffer.buf_read(read_ptr, sizeof(uint32_t));
  if (len < sizeof(uint32_t) || read_ptr == nullptr) return false;
  output = *reinterpret_cast<uint32_t*>(read_ptr);
  return true;
}

bool read_message(io_buf& buffer, uint32_t& message_type_out, std::vector<uint8_t>& payload_out)
{
  message_type_out = 0;
  payload_out.clear();

  if (!read_uint32(buffer, message_type_out))
  {
    // end of file
    return false;
  }

  uint32_t size = 0;
  BOOST_TEST(read_uint32(buffer, size));

  if (message_type_out == MSG_TYPE_FILEMAGIC)
  {
    // for file magic message, size is actually version number
    BOOST_CHECK_EQUAL(size, BINARY_PARSER_VERSION);
    return true;
  }

  // if not file magic, must be another valid message type
  BOOST_CHECK(message_type_out == MSG_TYPE_HEADER || message_type_out == MSG_TYPE_REGULAR ||
      message_type_out == MSG_TYPE_CHECKPOINT || message_type_out == MSG_TYPE_EOF);

  // all other message types have a real payload
  char* read_ptr = nullptr;
  auto actual_bytes_read = buffer.buf_read(read_ptr, size);
  BOOST_CHECK_EQUAL(actual_bytes_read, size);
  payload_out.insert(payload_out.begin(), read_ptr, read_ptr + size);

  // read padding bytes
  auto padding_size = size % 8;
  actual_bytes_read = buffer.buf_read(read_ptr, padding_size);
  BOOST_CHECK_EQUAL(actual_bytes_read, padding_size);
  return true;
}

std::vector<test_event_batch> parse_joined_log(io_buf& buffer)
{
  uint32_t message_type;
  std::vector<uint8_t> payload;

  // must begin with file magic message
  BOOST_TEST(read_message(buffer, message_type, payload), "could not read file magic message at start of binary log");
  BOOST_CHECK_EQUAL(message_type, MSG_TYPE_FILEMAGIC);
  BOOST_TEST(payload.empty(), "file magic message has non-empty payload");

  std::vector<test_event_batch> output;
  while (read_message(buffer, message_type, payload))
  {
    if (message_type == MSG_TYPE_REGULAR)
    {
      auto joined_payload = flatbuffers::GetRoot<fbv2::JoinedPayload>(payload.data());
      auto verifier = flatbuffers::Verifier(payload.data(), payload.size());
      BOOST_TEST(joined_payload->Verify(verifier), "verification failed on JoinedPayload flatbuffer");

      test_event_batch batch;
      auto joined_payload_events = joined_payload->events();
      for (auto joined_event : *joined_payload_events)
      {
        auto event_fb = flatbuffers::GetRoot<fbv2::Event>(joined_event->event()->data());
        BOOST_CHECK_NE(event_fb->payload(), nullptr);
        BOOST_CHECK_NE(event_fb->meta(), nullptr);
        BOOST_CHECK_NE(event_fb->meta()->id(), nullptr);
        BOOST_CHECK_NE(event_fb->meta()->client_time_utc(), nullptr);
        BOOST_TEST(batch.add_event(test_event(event_fb)), "tried to insert duplicate event into event batch");
      }
      BOOST_TEST(batch.verify(), "event batch was empty or has bad event id");
      output.push_back(std::move(batch));
    }

    if (message_type == MSG_TYPE_EOF) { break; }
  }
  return output;
}

utility::configuration get_test_config()
{
  utility::configuration config;
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  config.set(name::PROTOCOL_VERSION, "2");
  config.set(name::EUD_DURATION, "0:0:0");
  return config;
}

}  // namespace
