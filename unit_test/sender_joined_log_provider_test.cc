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

#include <chrono>
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
  return std::tie(te1._event_id, te1._time, te1._payload_type, te1._payload) <
      std::tie(te2._event_id, te2._time, te2._payload_type, te2._payload);
}

inline bool operator==(const test_event& te1, const test_event& te2)
{
  return std::tie(te1._event_id, te1._time, te1._payload_type, te1._payload) ==
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

  bool operator==(const std::set<test_event>& other) { return _batch == other; }
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
  logger::preamble pre;
  pre.reserved = 0;
  pre.version = 0;
  pre.msg_type = logger::message_type::fb_generic_event_collection;
  pre.msg_size = static_cast<uint32_t>(event_batch_buffer.size());
  auto data_buffer = std::make_shared<utility::data_buffer>(event_batch_buffer.size());
  BOOST_TEST(pre.write_to_bytes(data_buffer->preamble_begin(), data_buffer->preamble_size()));
  std::memcpy(data_buffer->body_begin(), event_batch_buffer.data(), event_batch_buffer.size());
  data_buffer->set_body_endoffset(data_buffer->get_body_beginoffset() + event_batch_buffer.size());
  return data_buffer;
}

bool read_uint32(io_buf& buffer, uint32_t& output)
{
  char* read_ptr = nullptr;
  auto len = buffer.buf_read(read_ptr, sizeof(uint32_t));
  if (len != sizeof(uint32_t) || read_ptr == nullptr) return false;
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
  BOOST_TEST(read_uint32(buffer, size), "could not read payload size");

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

std::vector<test_event_batch> parse_joined_log(std::unique_ptr<VW::io::reader>&& joined_log)
{
  io_buf buffer;
  buffer.add_file(std::move(joined_log));

  // must begin with file magic message
  uint32_t message_type;
  std::vector<uint8_t> payload;
  BOOST_TEST(read_message(buffer, message_type, payload), "could not read file magic message at start of binary log");
  BOOST_CHECK_EQUAL(message_type, MSG_TYPE_FILEMAGIC);
  BOOST_TEST(payload.empty(), "file magic message has non-empty payload");

  std::vector<test_event_batch> output;
  while (read_message(buffer, message_type, payload))
  {
    if (message_type == MSG_TYPE_REGULAR)
    {
      auto joined_payload = flatbuffers::GetRoot<fbv2::JoinedPayload>(payload.data());
      auto joined_payload_verifier = flatbuffers::Verifier(payload.data(), payload.size());
      BOOST_TEST(joined_payload->Verify(joined_payload_verifier), "verification failed on JoinedPayload flatbuffer");

      test_event_batch batch;
      auto joined_payload_events = joined_payload->events();
      BOOST_CHECK_NE(joined_payload_events, nullptr);

      for (auto joined_event : *joined_payload_events)
      {
        BOOST_CHECK_NE(joined_event, nullptr);
        BOOST_CHECK_NE(joined_event->event(), nullptr);
        auto event_fb = flatbuffers::GetRoot<fbv2::Event>(joined_event->event()->data());
        auto event_verifier = flatbuffers::Verifier(joined_event->event()->data(), joined_event->event()->size());
        BOOST_TEST(event_fb->Verify(event_verifier), "verification failed on Event flatbuffer");

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

std::unique_ptr<sender_joined_log_provider> create_test_object()
{
  utility::configuration config;
  config.set(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  config.set(name::PROTOCOL_VERSION, "2");
  config.set(name::JOINER_EUD_DURATION, "0:0:10");  // EUD set to 10 seconds for all tests here

  std::unique_ptr<sender_joined_log_provider> sjlp;
  BOOST_CHECK_EQUAL(sender_joined_log_provider::create(sjlp, config), error_code::success);
  return sjlp;
}

timestamp get_time(int seconds_from_now = 0)
{
  auto now = std::chrono::system_clock::now();
  auto time = now + std::chrono::seconds(seconds_from_now);
  return timestamp(time);
}

}  // namespace

BOOST_AUTO_TEST_CASE(empty_join)
{
  auto sjlp = create_test_object();
  std::unique_ptr<VW::io::reader> output;
  BOOST_CHECK_EQUAL(sjlp->invoke_join(output), error_code::success);

  auto result = parse_joined_log(std::move(output));
  BOOST_CHECK_EQUAL(result.size(), 0);
}

BOOST_AUTO_TEST_CASE(one_interaction_before_eud)
{
  auto sjlp = create_test_object();
  test_event evt("id", get_time(0), fbv2::PayloadType_CB, "test payload");
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt)), error_code::success);

  std::unique_ptr<VW::io::reader> output;
  BOOST_CHECK_EQUAL(sjlp->invoke_join(output), error_code::success);

  auto result = parse_joined_log(std::move(output));
  BOOST_CHECK_EQUAL(result.size(), 0);
}

BOOST_AUTO_TEST_CASE(one_interaction_after_eud)
{
  auto sjlp = create_test_object();
  test_event evt("id", get_time(-99), fbv2::PayloadType_CB, "test payload");
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt)), error_code::success);

  std::unique_ptr<VW::io::reader> output;
  BOOST_CHECK_EQUAL(sjlp->invoke_join(output), error_code::success);

  auto result = parse_joined_log(std::move(output));
  BOOST_CHECK_EQUAL(result.size(), 1);
  BOOST_CHECK(result[0] == (std::set<test_event>{evt}));
}

BOOST_AUTO_TEST_CASE(one_interaction_with_observations)
{
  auto sjlp = create_test_object();
  test_event evt1("id", get_time(-20), fbv2::PayloadType_CB, "test payload");
  test_event evt2("id", get_time(-19), fbv2::PayloadType_Outcome, "observation 1");
  test_event evt3("id", get_time(-15), fbv2::PayloadType_Outcome, "observation 2");
  test_event evt4("id", get_time(-5), fbv2::PayloadType_Outcome, "this observation is past eud");
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt1)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt2)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt3)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt4)), error_code::success);

  std::unique_ptr<VW::io::reader> output;
  BOOST_CHECK_EQUAL(sjlp->invoke_join(output), error_code::success);

  auto result = parse_joined_log(std::move(output));
  BOOST_CHECK_EQUAL(result.size(), 1);
  BOOST_CHECK(result[0] == (std::set<test_event>{evt1, evt2, evt3}));
}

BOOST_AUTO_TEST_CASE(multiple_interactions_and_observations)
{
  auto sjlp = create_test_object();
  test_event evt1("id_0", get_time(-20), fbv2::PayloadType_CB, "test payload");
  test_event evt2("id_0", get_time(-19), fbv2::PayloadType_Outcome, "observation 1");
  test_event evt3("id_0", get_time(-15), fbv2::PayloadType_Outcome, "observation 2");
  test_event evt4("id_0", get_time(-5), fbv2::PayloadType_Outcome, "this observation is past eud");
  test_event evt5("id_1", get_time(-80), fbv2::PayloadType_CB, "test payload");
  test_event evt6("id_2", get_time(-50), fbv2::PayloadType_CB, "test payload");
  test_event evt7("id_2", get_time(-49), fbv2::PayloadType_Outcome, "observation 1");
  test_event evt8("id_3", get_time(-1), fbv2::PayloadType_CB, "this event is before eud");
  test_event evt9("id_4", get_time(-15), fbv2::PayloadType_CB, "test payload");

  // add events in order of time
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt5)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt6)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt7)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt1)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt2)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt9)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt3)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt4)), error_code::success);
  BOOST_CHECK_EQUAL(sjlp->receive_events(create_message(evt8)), error_code::success);

  std::unique_ptr<VW::io::reader> output;
  BOOST_CHECK_EQUAL(sjlp->invoke_join(output), error_code::success);

  auto result = parse_joined_log(std::move(output));
  BOOST_CHECK_EQUAL(result.size(), 4);
  BOOST_CHECK(result[0] == (std::set<test_event>{evt5}));
  BOOST_CHECK(result[1] == (std::set<test_event>{evt6, evt7}));
  BOOST_CHECK(result[2] == (std::set<test_event>{evt1, evt2, evt3}));
  BOOST_CHECK(result[3] == (std::set<test_event>{evt9}));
}
