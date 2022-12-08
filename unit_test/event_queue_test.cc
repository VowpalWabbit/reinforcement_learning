#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "logger/event_queue.h"
#include <boost/test/unit_test.hpp>

#include "data_buffer.h"
#include "err_constants.h"

#include <functional>
#include <thread>

using namespace reinforcement_learning;
using namespace std;

class test_event : public event
{
public:
  test_event() {}
  test_event(const string& id) : event(id.c_str(), timestamp{}) {}

  test_event(test_event&& other) noexcept : event(std::move(other)) {}
  test_event& operator=(test_event&& other) noexcept
  {
    if (&other != this) event::operator=(std::move(other));
    return *this;
  }

  bool try_drop(float drop_prob, int _drop_pass) override { return _seed_id.substr(0, 4) == "drop"; }

  std::string get_event_id() { return _seed_id; }
};

using Func = std::function<int(test_event&, api_status*)>;
using namespace std::placeholders;
int passthru(test_event& out_evt, api_status*, const std::shared_ptr<test_event>& evt_sp)
{
  out_evt = std::move(*evt_sp);
  return reinforcement_learning::error_code::success;
}

BOOST_AUTO_TEST_CASE(push_pop_test)
{
  event_queue<test_event> queue(30, events_counter_status::ENABLE);

  std::vector<std::string> vs = {"1", "2", "3"};
  for (int i = 0; i < vs.size(); ++i)
  {
    auto evt_sp = std::make_shared<test_event>(vs[i]);
    queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
  }

  Func f;
  test_event val;

  BOOST_CHECK_EQUAL(queue.size(), 3);
  queue.pop(&f);
  f(val, nullptr);
  BOOST_CHECK_EQUAL(val.get_event_id(), "1");
  BOOST_CHECK_EQUAL(val.get_event_index(), 1);

  BOOST_CHECK_EQUAL(queue.size(), 2);
  queue.pop(&f);
  f(val, nullptr);
  BOOST_CHECK_EQUAL(val.get_event_id(), "2");
  BOOST_CHECK_EQUAL(val.get_event_index(), 2);

  BOOST_CHECK_EQUAL(queue.size(), 1);
  queue.pop(&f);
  f(val, nullptr);
  BOOST_CHECK_EQUAL(val.get_event_id(), "3");
  BOOST_CHECK_EQUAL(val.get_event_index(), 3);
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(prune_test)
{
  event_queue<test_event> queue(30, events_counter_status::ENABLE);
  {
    auto evt_sp = std::make_shared<test_event>("no_drop_1");
    queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
  }
  {
    auto evt_sp = std::make_shared<test_event>("drop_1");
    queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
  }

  BOOST_CHECK_EQUAL(queue.size(), 2);
  BOOST_CHECK_EQUAL(queue.capacity(), 20);
  queue.prune(1.0);  // drop should not work since current capacity is less than limit (20 < 30)
  BOOST_CHECK_EQUAL(queue.size(), 2);
  BOOST_CHECK_EQUAL(queue.capacity(), 20);

  {
    auto evt_sp = std::make_shared<test_event>("no_drop_2");
    queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
  }
  {
    auto evt_sp = std::make_shared<test_event>("drop_2");
    queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
  }
  {
    auto evt_sp = std::make_shared<test_event>("no_drop_3");
    queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
  }

  Func f;
  test_event val;

  BOOST_CHECK_EQUAL(queue.size(), 5);
  BOOST_CHECK_EQUAL(queue.capacity(), 50);
  queue.prune(1.0);  // drop should work since current capacity is more than limit (50 > 30)
  BOOST_CHECK_EQUAL(queue.size(), 3);
  BOOST_CHECK_EQUAL(queue.capacity(), 30);

  queue.pop(&f);
  f(val, nullptr);
  BOOST_CHECK_EQUAL(val.get_event_id(), "no_drop_1");
  BOOST_CHECK_EQUAL(val.get_event_index(), 1);

  queue.pop(&f);
  f(val, nullptr);
  BOOST_CHECK_EQUAL(val.get_event_id(), "no_drop_2");
  BOOST_CHECK_EQUAL(val.get_event_index(), 3);

  queue.pop(&f);
  f(val, nullptr);
  BOOST_CHECK_EQUAL(val.get_event_id(), "no_drop_3");
  BOOST_CHECK_EQUAL(val.get_event_index(), 5);
}

BOOST_AUTO_TEST_CASE(queue_push_pop)
{
  reinforcement_learning::event_queue<test_event> queue(30);

  // push n elements in the queue
  int n = 10;
  for (int i = 0; i < n; ++i)
  {
    auto evt_sp = std::make_shared<test_event>(std::to_string(i + 1));
    queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
  }

  BOOST_CHECK_EQUAL(queue.size(), n);

  // pop front
  Func f;
  test_event item;
  queue.pop(&f);
  f(item, nullptr);
  BOOST_CHECK_EQUAL(item.get_event_id(), std::string("1"));

  // pop all
  while (queue.size() > 0) queue.pop(&f);

  // check last item
  f(item, nullptr);
  BOOST_CHECK_EQUAL(item.get_event_id(), std::to_string(n));

  // check queue size
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(queue_push_pop_subsample)
{
  reinforcement_learning::event_queue<test_event> queue(30, events_counter_status::ENABLE, 0.5);

  // push n elements in the queue
  int n = 10;
  for (int i = 0; i < n; ++i)
  {
    std::string id;
    if (i % 2 == 0) { id = "drop_" + std::to_string(i + 1); }
    else
    {
      id = "no_drop_" + std::to_string(i + 1);
    }

    auto evt_sp = std::make_shared<test_event>(id);
    queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
  }

  BOOST_CHECK_EQUAL(queue.size(), n / 2);

  Func f;
  test_event item;

  // pop all
  for (int i = 0; i < n; ++i)
  {
    if (i % 2 != 0)
    {
      queue.pop(&f);
      f(item, nullptr);
      BOOST_CHECK_EQUAL(item.get_event_id(), "no_drop_" + std::to_string(i + 1));
      BOOST_CHECK_EQUAL(item.get_event_index(), i + 1);
    }
  }
}

BOOST_AUTO_TEST_CASE(queue_push_pop_threads)
{
  reinforcement_learning::event_queue<test_event> queue(30, events_counter_status::ENABLE);
  std::vector<thread> _threads;

  int n = 10;
  for (int i = 0; i < n; ++i)
  {
    _threads.push_back(thread([&queue, i] {
      auto evt_sp = std::make_shared<test_event>(std::to_string(i + 1));
      queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
    }));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));  // safe timeout
  }

  for (int i = 0; i < n; ++i) { _threads[i].join(); }

  BOOST_CHECK_EQUAL(queue.size(), n);
  Func f;
  test_event item;
  // pop front
  for (int i = 0; i < n; ++i)
  {
    queue.pop(&f);
    f(item, nullptr);
    BOOST_CHECK_EQUAL(item.get_event_id(), std::to_string(i + 1));
    BOOST_CHECK_EQUAL(item.get_event_index(), i + 1);
  }
}

BOOST_AUTO_TEST_CASE(queue_push_pop_threads_subsampling)
{
  reinforcement_learning::event_queue<test_event> queue(30, events_counter_status::ENABLE, 0.5);
  std::vector<thread> _threads;

  int n = 10;
  for (int i = 0; i < n; ++i)
  {
    if (i % 2 == 0)
    {
      _threads.push_back(thread([&queue, i] {
        auto evt_sp = std::make_shared<test_event>("drop_" + std::to_string(i + 1));
        queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
      }));
    }
    else
    {
      _threads.push_back(thread([&queue, i] {
        auto evt_sp = std::make_shared<test_event>("no_drop_" + std::to_string(i + 1));
        queue.push(std::bind(passthru, _1, _2, evt_sp), 10, evt_sp.get());
      }));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));  // safe timeout
  }

  for (int i = 0; i < n; ++i) { _threads[i].join(); }

  BOOST_CHECK_EQUAL(queue.size(), n / 2);

  Func f;
  test_event item;

  // pop all
  for (int i = 0; i < n; ++i)
  {
    if (i % 2 != 0)
    {
      queue.pop(&f);
      f(item, nullptr);
      BOOST_CHECK_EQUAL(item.get_event_id(), "no_drop_" + std::to_string(i + 1));
      BOOST_CHECK_EQUAL(item.get_event_index(), i + 1);
    }
  }
}
BOOST_AUTO_TEST_CASE(queue_pop_empty)
{
  reinforcement_learning::event_queue<test_event> queue(30);

  // the pop call on an empty queue should do nothing
  Func* f = NULL;
  queue.pop(f);
  if (f) BOOST_ERROR("item should be null");
}

BOOST_AUTO_TEST_CASE(queue_capacity_test)
{
  auto test = std::make_shared<test_event>("hello");
  reinforcement_learning::event_queue<test_event> queue(30);

  BOOST_CHECK_EQUAL(queue.capacity(), 0);
  // Contents of string moved into queue
  queue.push(std::bind(passthru, _1, _2, test), 10, test.get());
  BOOST_CHECK_EQUAL(queue.capacity(), 10);

  // Contents of queue string moved into passed in string
  Func f;
  queue.pop(&f);
  BOOST_CHECK_EQUAL(queue.capacity(), 0);
}