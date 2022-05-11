#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>
#include <memory>
#include "data_buffer.h"
#include "err_constants.h"
#include "serialization/json_serializer.h"
#include "logger/async_batcher.h"
#include "sender.h"
#include "vw/core/vw_math.h"

using namespace reinforcement_learning;
//This class simply implement a 'send' method, in order to be used as a template in the async_batcher
class message_sender : public logger::i_message_sender {
  std::vector<std::string>& items;
public:
  explicit message_sender(std::vector<std::string>& _items)
    : items(_items), sender{nullptr} {}

  int send(const uint16_t msg_type, const buffer& db, api_status* status = nullptr) override {
    items.emplace_back(reinterpret_cast<char*>(db->body_begin()));
    return error_code::success;
  };
  int init(api_status* status) override { return error_code::success; };
  i_sender* sender;
};

class test_undroppable_event : public event {
public:
  test_undroppable_event() {}

  test_undroppable_event(const std::string& id)
    : event(id.c_str(), timestamp{}) {}

  test_undroppable_event(test_undroppable_event&& other) = default;

  test_undroppable_event& operator=(test_undroppable_event&& other) = default;

  bool try_drop(float drop_prob, int _drop_pass) override { return false; }
  std::string get_event_id() { return _seed_id; }
};

class test_droppable_event : public event {
public:
  test_droppable_event() {}

  test_droppable_event(const std::string& id)
    : event(id.c_str(), timestamp{}) {}

  test_droppable_event(test_droppable_event&& other) = default;

  test_droppable_event& operator=(test_droppable_event&& other) = default;

  bool try_drop(float drop_prob, int _drop_pass) override { return true; }
  std::string get_event_id() { return _seed_id; }
};

// event that converts the event_id into a float used for the drop probability
class config_drop_event : public event {
public:
  config_drop_event() {}
  config_drop_event(const std::string& id) : event(id.c_str(), timestamp{}) {}

  config_drop_event(config_drop_event&& other) = default;
  config_drop_event& operator=(config_drop_event&& other) = default;

  bool try_drop(float drop_prob, int _drop_pass) override {
    auto prob = static_cast<float>(std::atof(_seed_id.c_str()));
    // logic because floating point comparison is dumb. In this case, atof(0.7) > 0.7 == true
    return (prob > drop_prob) && !VW::math::are_same(prob, drop_prob);
  }
  std::string get_event_id() { return _seed_id; }
};

namespace reinforcement_learning { namespace logger {
  template <>
  struct json_event_serializer<test_droppable_event> {
    using serializer_t = json_event_serializer<test_droppable_event>;

    static int serialize(test_droppable_event& evt, std::ostream& out, api_status* status) {
      out << evt.get_seed_id();
      return error_code::success;
    }

    static size_t size_estimate(const test_droppable_event& evt) { return 1; }
  };
  template <>
  struct json_event_serializer<test_undroppable_event> {
    using serializer_t = json_event_serializer<test_undroppable_event>;

    static int serialize(test_undroppable_event& evt, std::ostream& out, api_status* status) {
      out << evt.get_event_id();
      return error_code::success;
    }

    static size_t size_estimate(const test_undroppable_event& evt) { return 1; }
  };

  template <>
  struct json_event_serializer<config_drop_event> {
    using serializer_t = json_event_serializer<config_drop_event>;

    static int serialize(config_drop_event& evt, std::ostream& out, api_status* status) {
      out << evt.get_event_id();
      return error_code::success;
    }

    static size_t size_estimate(const config_drop_event& evt) { return 1; }
  };
}}

void expect_no_error(const api_status& s, void* cntxt) {
  BOOST_ASSERT(s.get_error_code() == error_code::success);
  BOOST_FAIL("Should not get background error notifications");
}

//test the flush mechanism based on a timer
BOOST_AUTO_TEST_CASE(flush_timeout) {
  std::vector<std::string> items;
  auto s = new message_sender(items);
  size_t timeout_ms = 100; //set a short timeout
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog(nullptr);
  utility::async_batcher_config config;
  config.send_high_water_mark = 262143;
  config.send_batch_interval_ms = static_cast<int>(timeout_ms);
  config.send_queue_max_capacity = 8192;
  int dummy = 0;
  logger::async_batcher<test_undroppable_event> batcher(s, watchdog, dummy, &error_fn, config);
  batcher.init(nullptr); // Allow periodic_background_proc inside async_batcher to start waiting
  // on a timer before sending any events to it. Else we risk not
  // triggering the batch mechanism and might get triggered by initial
  // pass in do..while loop
  std::this_thread::sleep_for(std::chrono::milliseconds(20)); //add 2 items in the current batch
  std::string foo("foo");
  std::string bar("bar");

  {
    auto foo_evt_sp = std::make_shared<test_undroppable_event>(foo);
    auto evt_fn =
      [foo_evt_sp](test_undroppable_event& out_evt, api_status* status)->int {
        out_evt = std::move(*foo_evt_sp);
        return error_code::success;
      };
    batcher.append(std::move(evt_fn), foo_evt_sp.get(), nullptr);
  }
  {
    auto bar_evt_sp = std::make_shared<test_undroppable_event>(bar);
    auto evt_fn =
      [bar_evt_sp](test_undroppable_event& out_evt, api_status* status)->int {
        out_evt = std::move(*bar_evt_sp);
        return error_code::success;
      };
    batcher.append(std::move(evt_fn), bar_evt_sp.get(), nullptr);
  }
  std::string expected = foo + "\n" + bar + "\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  BOOST_REQUIRE_EQUAL(items.size(), 1);
  std::string result;
  for (auto item : items) { result.append(item.begin(), item.end()); }
  BOOST_CHECK_EQUAL(result, expected);
}

//test that the batcher split batches as expected
BOOST_AUTO_TEST_CASE(flush_batches) {
  std::vector<std::string> items;
  auto s = new message_sender(items);
  size_t send_high_water_mark = 10; //bytes
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog(nullptr);
  utility::async_batcher_config config;
  config.send_high_water_mark = static_cast<int>(send_high_water_mark);
  config.send_batch_interval_ms = static_cast<int>(100000);
  int dummy = 0;
  auto batcher = new logger::async_batcher<test_undroppable_event>
      (s, watchdog, dummy, &error_fn, config);
  batcher->init(nullptr); // Allow periodic_background_proc inside async_batcher to start waiting
  // on a timer before sending any events to it.   Else we risk not
  // triggering the batch mechanism and might get triggered by initial
  // pass in do..while loop
  std::this_thread::sleep_for(std::chrono::milliseconds(20)); //add 2 items in the current batch
  std::string foo("foo");
  std::string bar("bar-yyy");

  {
    auto foo_evt_sp = std::make_shared<test_undroppable_event>(foo);
    auto evt_fn =
      [foo_evt_sp](test_undroppable_event& out_evt, api_status* status)->int {
        out_evt = std::move(*foo_evt_sp);
        return error_code::success;
      };
    batcher->append(std::move(evt_fn), foo_evt_sp.get(), nullptr);
  }
  {
    auto bar_evt_sp = std::make_shared<test_undroppable_event>(bar);
    auto evt_fn =
      [bar_evt_sp](test_undroppable_event& out_evt, api_status* status)->int {
        out_evt = std::move(*bar_evt_sp);
        return error_code::success;
      };
    batcher->append(std::move(evt_fn), bar_evt_sp.get(), nullptr);
  }
  //'send_high_water_mark' will be triggered by previous 2 items.
  //next item will be added in a new batch
  std::string hello("hello");
  {
    auto hello_evt_sp = std::make_shared<test_undroppable_event>(hello);
    auto evt_fn =
      [hello_evt_sp](test_undroppable_event& out_evt, api_status* status)->int {
        out_evt = std::move(*hello_evt_sp);
        return error_code::success;
      };
    batcher->append(std::move(evt_fn), hello_evt_sp.get(), nullptr);
  }

  const std::string expected_batch_0 = foo + "\n" + bar + "\n";
  const std::string expected_batch_1 = hello + "\n";
  delete batcher; //flush force
  BOOST_REQUIRE_EQUAL(items.size(), 2);
  BOOST_CHECK_EQUAL(items[0], expected_batch_0);
  BOOST_CHECK_EQUAL(items[1], expected_batch_1);
}

//test that the batcher flushes everything before deletion
BOOST_AUTO_TEST_CASE(flush_after_deletion) {
  std::vector<std::string> items;
  auto s = new message_sender(items);
  utility::watchdog watchdog(nullptr);
  utility::async_batcher_config config;
  int dummy = 0;
  auto* batcher = new logger::async_batcher<test_undroppable_event>
      (s, watchdog, dummy, nullptr, config);
  batcher->init(nullptr); // Allow periodic_background_proc to start waiting
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  std::string foo("foo");
  std::string bar("bar");
  {
    auto foo_evt_sp = std::make_shared<test_undroppable_event>(foo);
    auto evt_fn =
      [foo_evt_sp](test_undroppable_event& out_evt, api_status* status)->int {
        out_evt = std::move(*foo_evt_sp);
        return error_code::success;
      };
    batcher->append(std::move(evt_fn), foo_evt_sp.get(), nullptr);
  }
  {
    auto bar_evt_sp = std::make_shared<test_undroppable_event>(bar);
    auto evt_fn =
      [bar_evt_sp](test_undroppable_event& out_evt, api_status* status)->int {
        out_evt = std::move(*bar_evt_sp);
        return error_code::success;
      };
    batcher->append(std::move(evt_fn), bar_evt_sp.get(), nullptr);
  }
                                                //batch was not sent yet
  BOOST_CHECK_EQUAL(items.size(), 0);           //batch flush is triggered on delete
  delete batcher;                               //check the batch was sent
  BOOST_REQUIRE_EQUAL(items.size(), 1);
  std::string expected = foo + "\n" + bar + "\n";
  BOOST_CHECK_EQUAL(items[0], expected);
}

//test that events are not dropped using the queue_dropping_disable option, even if the queue max capacity is reached
BOOST_AUTO_TEST_CASE(queue_overflow_do_not_drop_event) {
  std::vector<std::string> items;
  auto s = new message_sender(items);
  size_t timeout_ms = 100;
  size_t queue_max_size = 3;
  queue_mode_enum queue_mode = queue_mode_enum::BLOCK;
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog(nullptr);
  utility::async_batcher_config config;
  config.send_high_water_mark = 262143;
  config.send_batch_interval_ms = static_cast<int>(timeout_ms);
  config.send_queue_max_capacity = static_cast<int>(queue_max_size);
  config.queue_mode = queue_mode;
  int dummy = 0;
  auto batcher = new logger::async_batcher<test_droppable_event>(s, watchdog, dummy, &error_fn, config);
  batcher->init(nullptr); // Allow periodic_background_proc to start waiting
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  int n = 10;
  for (int i = 0; i < n; ++i) {
    auto evt_sp = std::make_shared<test_droppable_event>(std::to_string(i));
    auto evt_fn =
      [evt_sp](test_droppable_event& out_evt, api_status* status)->int {
        out_evt = std::move(*evt_sp);
        return error_code::success;
      };
    batcher->append(std::move(evt_fn), evt_sp.get(), nullptr);
  } //triggers a final flush
  delete batcher;
  //all batches were sent. Check that no event was dropped
  std::string expected_output = "0\n";
  for (int i = 1; i < n; ++i) {
    expected_output += std::to_string(i);
    expected_output.append("\n");
  }
  BOOST_REQUIRE(!items.empty());
  std::string actual_output;
  for (const auto& item : items) { actual_output.append(item); }
  BOOST_CHECK_EQUAL(expected_output, actual_output);
}

BOOST_AUTO_TEST_CASE(queue_config_drop_rate_test)
{
  std::vector<std::string> items;
  auto s = new message_sender(items);
  size_t timeout_ms = 100;
  size_t queue_max_size = 10;
  queue_mode_enum queue_mode = queue_mode_enum::BLOCK;
  error_callback_fn error_fn(expect_no_error, nullptr);
  utility::watchdog watchdog(nullptr);
  utility::async_batcher_config config;
  config.send_high_water_mark = 262143;
  config.send_batch_interval_ms = static_cast<int>(timeout_ms);
  config.send_queue_max_capacity = static_cast<int>(queue_max_size);
  config.queue_mode = queue_mode;
  config.subsample_rate = 0.7f;
  int dummy = 0;
  auto batcher = new logger::async_batcher<config_drop_event>(s, watchdog, dummy, &error_fn, config);
  batcher->init(nullptr);
  // Allow periodic_background_proc to start waiting
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  std::vector<std::string> vs = {"0.00", "1.00", "0.69", "0.70", "0.71"};
  for (int i = 0; i < vs.size(); ++i) {
    auto evt_sp = std::make_shared<config_drop_event>(vs[i]);
    auto evt_fn =
      [evt_sp](config_drop_event& out_evt, api_status* status)->int {
        out_evt = std::move(*evt_sp);
        return error_code::success;
      };
    batcher->append(std::move(evt_fn), evt_sp.get(), nullptr);
  }
  delete batcher;

  BOOST_REQUIRE(!items.empty());
  BOOST_CHECK_EQUAL(items[0], "0.00\n0.69\n0.70\n");
}

BOOST_AUTO_TEST_CASE(get_batcher_config_counter_status_test)
{
    utility::configuration config;
    utility::async_batcher_config batcher_config = utility::get_batcher_config(config, INTERACTION_SECTION);
    BOOST_ASSERT(batcher_config.event_counter_status == events_counter_status::DISABLE);
    config.set("interaction.sender.implementation", "INTERACTION_HTTP_API_SENDER");
    batcher_config = utility::get_batcher_config(config, INTERACTION_SECTION);
    BOOST_ASSERT(batcher_config.event_counter_status == events_counter_status::DISABLE);
    config.set("protocol.version", "2");
    batcher_config = utility::get_batcher_config(config, INTERACTION_SECTION);
    BOOST_ASSERT(batcher_config.event_counter_status == events_counter_status::ENABLE);
    config.set("interaction.sender.implementation", "INTERACTION_FILE_SENDER");
    batcher_config = utility::get_batcher_config(config, INTERACTION_SECTION);
    BOOST_ASSERT(batcher_config.event_counter_status == events_counter_status::DISABLE);
    config.set("interaction.sender.implementation", "INTERACTION_EH_SENDER");
    batcher_config = utility::get_batcher_config(config, INTERACTION_SECTION);
    BOOST_ASSERT(batcher_config.event_counter_status == events_counter_status::DISABLE);
    config.set("observation.sender.implementation", "OBSERVATION_EH_SENDER");
    batcher_config = utility::get_batcher_config(config, OBSERVATION_SECTION);
    BOOST_ASSERT(batcher_config.event_counter_status == events_counter_status::DISABLE);
    config.set("observation.sender.implementation", "OBSERVATION_FILE_SENDER");
    batcher_config = utility::get_batcher_config(config, OBSERVATION_SECTION);
    BOOST_ASSERT(batcher_config.event_counter_status == events_counter_status::DISABLE);
    config.set("observation.sender.implementation", "OBSERVATION_HTTP_API_SENDER");
    batcher_config = utility::get_batcher_config(config, OBSERVATION_SECTION);
    BOOST_ASSERT(batcher_config.event_counter_status == events_counter_status::DISABLE);
}
