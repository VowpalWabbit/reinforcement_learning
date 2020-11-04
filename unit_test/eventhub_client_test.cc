#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "err_constants.h"
#include "logger/eventhub_client.h"
#include "logger/preamble.h"
#include "mock_http_client.h"
#include "utility/data_buffer_streambuf.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;

class error_counter
{
public:
  void _error_handler(void)
  {
    _err_count++;
  }

  int _err_count = 0;
};

void error_counter_func(const r::api_status &, void *counter)
{
  static_cast<error_counter *>(counter)->_error_handler();
}

BOOST_AUTO_TEST_CASE(send_something)
{
  // TODO: Use create_http_client with mock client option in configuration.
  auto http_client = new mock_http_client("localhost:8080");

  //create a client
  r::eventhub_client eh(http_client, "localhost:8080", "", "", "", 1, 1, nullptr, nullptr);
  r::api_status ret;

  std::shared_ptr<u::data_buffer> db1(new u::data_buffer());
  u::data_buffer_streambuf sbuff1(db1.get());
  std::ostream message1(&sbuff1);

  message1 << "message 1";

  std::shared_ptr<u::data_buffer> db2(new u::data_buffer());
  u::data_buffer_streambuf sbuff2(db2.get());
  std::ostream message2(&sbuff2);

  message2 << "message 2";

  //send events
  sbuff1.finalize();
  sbuff2.finalize();
  BOOST_CHECK_EQUAL(eh.send(db1, &ret), r::error_code::success);
  BOOST_CHECK_EQUAL(eh.send(db2, &ret), r::error_code::success);
}

BOOST_AUTO_TEST_CASE(retry_http_send_success)
{
  auto http_client = new mock_http_client("localhost:8080");

  int tries = 0;
  int succeed_after_n_tries = 3;
  http_client->set_responder(
      r::http_method::POST,
      [&tries, succeed_after_n_tries](const r::http_request &) {
        tries++;

        if (tries > succeed_after_n_tries)
          return std::unique_ptr<r::http_response>(
              new mock_http_response(201 /*Created*/));

        return std::unique_ptr<r::http_response>(
            new mock_http_response(500 /*InternalError*/));
      });

  error_counter counter;
  reinforcement_learning::error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    reinforcement_learning::eventhub_client eh(http_client, "localhost:8080", "", "", "", 1, 8 /* retries */, nullptr, &error_callback);
    reinforcement_learning::api_status ret;

    std::shared_ptr<u::data_buffer> db1(new u::data_buffer());
    u::data_buffer_streambuf sbuff1(db1.get());
    std::ostream message1(&sbuff1);

    message1 << "message 1";
    BOOST_CHECK_EQUAL(eh.send(db1, &ret), r::error_code::success);
  }

  // Although it was allowed to retry 8 times, it should stop after succeeding at 4.
  BOOST_CHECK_EQUAL(tries, succeed_after_n_tries + 1);
  BOOST_CHECK_EQUAL(counter._err_count, 0);
}

BOOST_AUTO_TEST_CASE(retry_http_send_fail)
{
  auto http_client = new mock_http_client("localhost:8080");

  const int MAX_RETRIES = 10;

  int tries = 0;
  http_client->set_responder(
      r::http_method::POST,
      [&tries](const r::http_request &) {
        tries++;
        return std::unique_ptr<r::http_response>(
            new mock_http_response(500 /*InternalError*/));
      });

  error_counter counter;
  r::error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    r::eventhub_client eh(http_client, "localhost:8080", "", "", "", 1, MAX_RETRIES, nullptr, &error_callback);

    r::api_status ret;
    std::shared_ptr<u::data_buffer> db1(new u::data_buffer());
    u::data_buffer_streambuf sbuff1(db1.get());
    std::ostream message1(&sbuff1);

    message1 << "message 1";
    BOOST_CHECK_EQUAL(eh.send(db1, &ret), r::error_code::success);
  }

  BOOST_CHECK_EQUAL(tries, MAX_RETRIES + 1);
  BOOST_CHECK_EQUAL(counter._err_count, 1);
}

BOOST_AUTO_TEST_CASE(http_in_order_after_retry)
{
  auto *http_client = new mock_http_client("localhost:8080");

  const int MAX_RETRIES = 10;
  int tries = 0;
  std::vector<std::string> received_messages;
  http_client->set_responder(
      r::http_method::POST,
      [&tries, &received_messages](const r::http_request &request) {
        tries++;

        // Succeed every 4th attempt.
        if (tries >= 4)
        {
          tries = 0;
          const auto buffer = request._body_buffer;
          received_messages.push_back(std::string(buffer->body_begin(), buffer->body_begin() + buffer->body_filled_size()));
          return std::unique_ptr<r::http_response>(
              new mock_http_response(201 /*Created*/));
        }

        return std::unique_ptr<r::http_response>(new mock_http_response(500 /*InternalError*/));
      });

  error_counter counter;
  r::error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    r::eventhub_client eh(http_client, "localhost:8080", "", "", "", 1, MAX_RETRIES, nullptr, &error_callback);

    r::api_status ret;
    std::shared_ptr<u::data_buffer> db1(new u::data_buffer());
    u::data_buffer_streambuf sbuff1(db1.get());
    std::ostream message1(&sbuff1);
    message1 << std::unitbuf;
    message1 << "message 1";
    BOOST_CHECK_EQUAL(eh.send(db1, &ret), r::error_code::success);

    std::shared_ptr<u::data_buffer> db2(new u::data_buffer());
    u::data_buffer_streambuf sbuff2(db2.get());
    std::ostream message2(&sbuff2);

    message2 << std::unitbuf;
    message2 << "message 2";
    BOOST_CHECK_EQUAL(eh.send(db2, &ret), r::error_code::success);

    std::shared_ptr<u::data_buffer> db3(new u::data_buffer());
    u::data_buffer_streambuf sbuff3(db3.get());
    std::ostream message3(&sbuff3);

    message3 << std::unitbuf;
    message3 << "message 3";
    BOOST_CHECK_EQUAL(eh.send(db3, &ret), r::error_code::success);

    std::shared_ptr<u::data_buffer> db4(new u::data_buffer());
    u::data_buffer_streambuf sbuff4(db4.get());
    std::ostream message4(&sbuff4);

    message4 << std::unitbuf;
    message4 << "message 4";
    BOOST_CHECK_EQUAL(eh.send(db4, &ret), r::error_code::success);

    std::shared_ptr<u::data_buffer> db5(new u::data_buffer());
    u::data_buffer_streambuf sbuff5(db5.get());
    std::ostream message5(&sbuff5);

    message5 << std::unitbuf;
    message5 << "message 5";
    BOOST_CHECK_EQUAL(eh.send(db5, &ret), r::error_code::success);
  }

  BOOST_CHECK_EQUAL(received_messages[0], "message 1");
  BOOST_CHECK_EQUAL(received_messages[1], "message 2");
  BOOST_CHECK_EQUAL(received_messages[2], "message 3");
  BOOST_CHECK_EQUAL(received_messages[3], "message 4");
  BOOST_CHECK_EQUAL(received_messages[4], "message 5");
  BOOST_CHECK_EQUAL(counter._err_count, 0);
}
