#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>


#include "data_buffer.h"
#include "err_constants.h"

#include "serialization/json_serializer.h"
#include "logger/async_batcher.h"
#include "sender.h"

using namespace reinforcement_learning;

//This class simply implement a 'send' method, in order to be used as a template in the async_batcher
class message_sender : public logger::i_message_sender {
  std::vector<std::string>& items;

public:
  explicit message_sender(std::vector<std::string>& _items)
    : items(_items), sender{ nullptr }
  {}

  int send(const uint16_t msg_type, buffer& db, api_status* status = nullptr) override {
    items.emplace_back(reinterpret_cast<char*>(db->body_begin()));
    return error_code::success;
  };

  int init(api_status* status) override {
    return error_code::success;
  };

  i_sender* sender;
};

class test_undroppable_event : public event {
public:
  test_undroppable_event() {}
  test_undroppable_event(const std::string& id) : event(id.c_str()) {}

  test_undroppable_event(test_undroppable_event&& other) : event(std::move(other)) {}
  test_undroppable_event& operator=(test_undroppable_event&& other)
  {
    if (&other != this) event::operator=(std::move(other));
    return *this;
  }

  bool try_drop(float drop_prob, int _drop_pass) override {
    return false;
  }

  std::string get_event_id() {
    return _event_id;
  }
};

class test_droppable_event : public event {
public:
  test_droppable_event() {}
  test_droppable_event(const std::string& id) : event(id.c_str()) {}
  test_droppable_event(test_droppable_event&& other) : event(std::move(other)) {}
  test_droppable_event& operator=(test_droppable_event&& other)
  {
    if (&other != this) event::operator=(std::move(other));
    return *this;
  }

  bool try_drop(float drop_prob, int _drop_pass) override {
    return true;
  }
};

namespace reinforcement_learning {
  namespace logger {
    template<>
    struct json_event_serializer<test_droppable_event> {
      using serializer_t = json_event_serializer<test_droppable_event>;
      static int serialize(test_droppable_event& evt, std::ostream& out, api_status* status) {
        out << evt.get_event_id();
        return error_code::success;
      }
      static size_t size_estimate(const test_droppable_event& evt) {
        return 1;
      }
    };

    template<>
    struct json_event_serializer<test_undroppable_event> {
      using serializer_t = json_event_serializer<test_undroppable_event>;
      static int serialize(test_undroppable_event& evt, std::ostream& out, api_status* status) {
        out << evt.get_event_id();
        return error_code::success;
      }
      static size_t size_estimate(const test_undroppable_event& evt) {
        return 1;
      }
    };
  }
}

void expect_no_error(const api_status& s, void* cntxt)
{
  BOOST_ASSERT(s.get_error_code() == error_code::success);
  BOOST_FAIL("Should not get background error notifications");
}

//test the flush mechanism based on a timer
BOOST_AUTO_TEST_CASE(flush_timeout)
{
  std::vector<std::string> items;
  auto s = new message_sender(items);
  size_t timeout_ms = 100; //set a short timeout
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog(nullptr);
  logger::async_batcher<test_undroppable_event> batcher(s, watchdog, &error_fn, 262143, timeout_ms, 8192);
  batcher.init(nullptr);

  // Allow periodic_background_proc inside async_batcher to start waiting
  // on a timer before sending any events to it. Else we risk not
  // triggering the batch mechanism and might get triggered by initial
  // pass in do..while loop
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  //add 2 items in the current batch
  std::string foo("foo");
  std::string bar("bar");
  batcher.append(test_undroppable_event(foo));
  batcher.append(test_undroppable_event(bar));

  //check the batch was sent
  std::string expected = foo + "\n" + bar + "\n";

  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  BOOST_REQUIRE_EQUAL(items.size(), 1);
  std::string result;
  for (auto item : items) {
    result.append(item.begin(), item.end());
  }
  BOOST_CHECK_EQUAL(result, expected);
}

//test that the batcher split batches as expected
BOOST_AUTO_TEST_CASE(flush_batches)
{
  std::vector<std::string> items;
  auto s = new message_sender(items);
  size_t send_high_water_mark = 10;//bytes
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog(nullptr);
  logger::async_batcher<test_undroppable_event>* batcher = new logger::async_batcher<test_undroppable_event>(s, watchdog, &error_fn, send_high_water_mark, 100000);
  batcher->init(nullptr);

  // Allow periodic_background_proc inside async_batcher to start waiting
  // on a timer before sending any events to it.   Else we risk not
  // triggering the batch mechanism and might get triggered by initial
  // pass in do..while loop
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  //add 2 items in the current batch
  std::string foo("foo");
  std::string bar("bar-yyy");
  batcher->append(test_undroppable_event(foo));   //3 bytes
  batcher->append(test_undroppable_event(bar));   //7 bytes

  //'send_high_water_mark' will be triggered by previous 2 items.
  //next item will be added in a new batch
  std::string hello("hello");
  batcher->append(test_undroppable_event(hello));

  std::string expected_batch_0 = foo + "\n" + bar + "\n";
  std::string expected_batch_1 = hello + "\n";

  delete batcher;//flush force

  BOOST_REQUIRE_EQUAL(items.size(), 2);
  BOOST_CHECK_EQUAL(items[0], expected_batch_0);
  BOOST_CHECK_EQUAL(items[1], expected_batch_1);
}

//test that the batcher flushes everything before deletion
BOOST_AUTO_TEST_CASE(flush_after_deletion)
{
  std::vector<std::string> items;
  auto s = new message_sender(items);
  utility::watchdog watchdog(nullptr);
  logger::async_batcher<test_undroppable_event>* batcher = new logger::async_batcher<test_undroppable_event>(s, watchdog);
  batcher->init(nullptr);

  // Allow periodic_background_proc to start waiting
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  std::string foo("foo");
  std::string bar("bar");
  batcher->append(test_undroppable_event(foo));
  batcher->append(test_undroppable_event(bar));

  //batch was not sent yet
  BOOST_CHECK_EQUAL(items.size(), 0);

  //batch flush is triggered on delete
  delete batcher;
  //check the batch was sent
  BOOST_REQUIRE_EQUAL(items.size(), 1);

  std::string expected = foo + "\n" + bar + "\n";
  BOOST_CHECK_EQUAL(items[0], expected);
}

//test that events are not dropped using the queue_dropping_disable option, even if the queue max capacity is reached
BOOST_AUTO_TEST_CASE(queue_overflow_do_not_drop_event)
{
  std::vector<std::string> items;
  auto s = new message_sender(items);
  size_t timeout_ms = 100;
  size_t queue_max_size = 3;
  queue_mode_enum queue_mode = BLOCK;
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog(nullptr);
  logger::async_batcher<test_droppable_event>* batcher = new logger::async_batcher<test_droppable_event>(s, watchdog, &error_fn, 262143, timeout_ms, queue_max_size, queue_mode);
  batcher->init(nullptr);

  // Allow periodic_background_proc to start waiting
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  int n = 10;
  for (int i = 0; i < n; ++i) {
    batcher->append(test_droppable_event(std::to_string(i)));
  }

  //triggers a final flush
  delete batcher;

  //all batches were sent. Check that no event was dropped
  std::string expected_output = "0\n";
  for (int i = 1; i < n; ++i) {
    expected_output += std::to_string(i);
    expected_output.append("\n");
  }

  BOOST_REQUIRE(items.size() > 0);
  std::string actual_output;
  for (auto item : items) {
    actual_output.append(item);
  }

  BOOST_CHECK_EQUAL(expected_output, actual_output);
}

BOOST_AUTO_TEST_CASE(convert_to_queue_mode_enum) {
  BOOST_CHECK_EQUAL(DROP, to_queue_mode_enum("DROP"));
  BOOST_CHECK_EQUAL(BLOCK, to_queue_mode_enum("BLOCK"));

  //default is DROP
  BOOST_CHECK_EQUAL(DROP, to_queue_mode_enum("something_else"));
}
