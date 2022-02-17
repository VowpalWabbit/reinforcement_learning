#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "data_buffer.h"
#include "logger/event_queue.h"
#include <boost/test/unit_test.hpp>

using namespace reinforcement_learning;
using namespace std;

class test_event : public event {
public:
  test_event() {}
  test_event(const string& id) : event(id.c_str(), timestamp{}) {}

  test_event(test_event&& other) : event(std::move(other)) {}
  test_event& operator=(test_event&& other)
  {
    if (&other != this) event::operator=(std::move(other));
    return *this;
  }

  bool try_drop(float drop_prob, int _drop_pass) override {
    return _seed_id.substr(0, 4) == "drop";
  }

  std::string get_event_id() {
    return _seed_id;
  }
};

BOOST_AUTO_TEST_CASE(push_pop_test) {
  event_queue<test_event> queue(30,events_counter_status::ENABLE);
  queue.push(test_event("1"),10);
  queue.push(test_event("2"),10);
  queue.push(test_event("3"),10);

  test_event val;

  BOOST_CHECK_EQUAL(queue.size(), 3);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.get_event_id(), "1");
  BOOST_CHECK_EQUAL(val.get_event_index(), 1);

  BOOST_CHECK_EQUAL(queue.size(), 2);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.get_event_id(), "2");
  BOOST_CHECK_EQUAL(val.get_event_index(), 2);

  BOOST_CHECK_EQUAL(queue.size(), 1);
  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.get_event_id(), "3");
  BOOST_CHECK_EQUAL(val.get_event_index(), 3);
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(prune_test) {
  event_queue<test_event> queue(30, events_counter_status::ENABLE);
  queue.push(test_event("no_drop_1"),10);
  queue.push(test_event("drop_1"),10);

  BOOST_CHECK_EQUAL(queue.size(), 2);
  BOOST_CHECK_EQUAL(queue.capacity(), 20);
  queue.prune(1.0); // drop should not work since current capacity is less than limit (20 < 30)
  BOOST_CHECK_EQUAL(queue.size(), 2);
  BOOST_CHECK_EQUAL(queue.capacity(), 20);

  queue.push(test_event("no_drop_2"),10);
  queue.push(test_event("drop_2"),10);
  queue.push(test_event("no_drop_3"),10);

  test_event val;

  BOOST_CHECK_EQUAL(queue.size(), 5);
  BOOST_CHECK_EQUAL(queue.capacity(), 50);
  queue.prune(1.0); // drop should work since current capacity is more than limit (50 > 30)
  BOOST_CHECK_EQUAL(queue.size(), 3);
  BOOST_CHECK_EQUAL(queue.capacity(), 30);


  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.get_event_id(), "no_drop_1");
  BOOST_CHECK_EQUAL(val.get_event_index(), 1);

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.get_event_id(), "no_drop_2");
  BOOST_CHECK_EQUAL(val.get_event_index(), 3);

  queue.pop(&val);
  BOOST_CHECK_EQUAL(val.get_event_id(), "no_drop_3");
  BOOST_CHECK_EQUAL(val.get_event_index(), 5);
}

BOOST_AUTO_TEST_CASE(queue_push_pop)
{
  reinforcement_learning::event_queue<test_event> queue(30);

  //push n elements in the queue
  int n = 10;
  for (int i=0; i<n; ++i)
      queue.push(test_event(std::to_string(i+1)),10);

  BOOST_CHECK_EQUAL(queue.size(), n);

  //pop front
  test_event item;
  queue.pop(&item);
  BOOST_CHECK_EQUAL(item.get_event_id(), std::string("1"));

  //pop all
  while (queue.size()>0)
      queue.pop(&item);

  //check last item
  BOOST_CHECK_EQUAL(item.get_event_id(), std::to_string(n));

  //check queue size
  BOOST_CHECK_EQUAL(queue.size(), 0);
}

BOOST_AUTO_TEST_CASE(queue_push_pop_subsample)
{
  reinforcement_learning::event_queue<test_event> queue(30,events_counter_status::ENABLE,0.5);
  
  //push n elements in the queue
  int n = 10;
  for (int i = 0; i < n; ++i) {
    if(i%2==0){ queue.push(test_event("drop_"+std::to_string(i + 1)), 10); }
    else{ queue.push(test_event("no_drop_"+std::to_string(i + 1)), 10); }
  }

  BOOST_CHECK_EQUAL(queue.size(), n/2);

  test_event item;
   
    //pop all
  for (int i = 0; i < n; ++i) {
    if (i % 2 != 0) {
      queue.pop(&item);
      BOOST_CHECK_EQUAL(item.get_event_id(), "no_drop_"+std::to_string(i + 1));
      BOOST_CHECK_EQUAL(item.get_event_index(), i + 1);
    }
  }
}

BOOST_AUTO_TEST_CASE(queue_push_pop_threads)
{
  reinforcement_learning::event_queue<test_event> queue(30, events_counter_status::ENABLE);
  std::vector<std::thread> _threads;

  int n = 10;
  for (int i = 0; i < n; ++i) {
    _threads.push_back(thread([&] { queue.push(test_event(std::to_string(i + 1)), 10); }));
    std::this_thread::sleep_for(std::chrono::milliseconds(5)); //safe timeout 
  }
    
  for (int i = 0; i < n; ++i) {
    _threads[i].join();
  }

  BOOST_CHECK_EQUAL(queue.size(), n);
  test_event item;
  //pop front
  for (int i = 0; i < n; ++i) {
    queue.pop(&item);
    BOOST_CHECK_EQUAL(item.get_event_id(), std::to_string(i + 1));
    BOOST_CHECK_EQUAL(item.get_event_index(), i + 1);
  }
}

BOOST_AUTO_TEST_CASE(queue_push_pop_threads_subsampling)
{
  reinforcement_learning::event_queue<test_event> queue(30, events_counter_status::ENABLE,0.5);
  std::vector<std::thread> _threads;

  int n = 10;
  for (int i = 0; i < n; ++i) {
    if (i % 2 == 0) { _threads.push_back(thread([&] { queue.push(test_event("drop_" + std::to_string(i + 1)), 10); })); }
    else { _threads.push_back(thread([&] { queue.push(test_event("no_drop_" + std::to_string(i + 1)), 10); })); }
    std::this_thread::sleep_for(std::chrono::milliseconds(5)); //safe timeout 
  }
    
  for (int i = 0; i < n; ++i) {
    _threads[i].join();
  }

  BOOST_CHECK_EQUAL(queue.size(), n/2);

  test_event item;
   
    //pop all
  for (int i = 0; i < n; ++i) {
    if (i % 2 != 0) {
      queue.pop(&item);
      BOOST_CHECK_EQUAL(item.get_event_id(), "no_drop_"+std::to_string(i + 1));
      BOOST_CHECK_EQUAL(item.get_event_index(), i + 1);
    }
  }
}
BOOST_AUTO_TEST_CASE(queue_pop_empty)
{
  reinforcement_learning::event_queue<test_event> queue(30);

  //the pop call on an empty queue should do nothing
  test_event* item = NULL;
  queue.pop(item);
  if (item)
      BOOST_ERROR("item should be null");
}

BOOST_AUTO_TEST_CASE(queue_move_push)
{
  test_event test("hello");
  reinforcement_learning::event_queue<test_event> queue(30);

  // Contents of string moved into queue
  queue.push(test,10);
  BOOST_CHECK_EQUAL(test.get_event_id(), "");

  // Contents of queue string moved into passed in string
  test_event item;
  queue.pop(&item);
  BOOST_CHECK_EQUAL(item.get_event_id(), "hello");
}

BOOST_AUTO_TEST_CASE(queue_capacity_test)
{
  test_event test("hello");
  reinforcement_learning::event_queue<test_event> queue(30);

  BOOST_CHECK_EQUAL(queue.capacity(), 0);
  // Contents of string moved into queue
  queue.push(test,10);
  BOOST_CHECK_EQUAL(queue.capacity(), 10);

  // Contents of queue string moved into passed in string
  test_event item;
  queue.pop(&item);
  BOOST_CHECK_EQUAL(queue.capacity(), 0);
}