#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "logger/eventhub_client.h"
#include "http_server/http_server.h"
#include <boost/test/unit_test.hpp>
#include "err_constants.h"

using namespace reinforcement_learning;

class error_counter {
public:
  void _error_handler(void) {
    _err_count++;
  }

  int _err_count = 0;
};

void error_counter_func(const api_status&, void* counter) {
  static_cast<error_counter*>(counter)->_error_handler();
}

BOOST_AUTO_TEST_CASE(send_something)
{
  mock_http_client* http_client = new mock_http_client("localhost:8080");

  //create a client
  eventhub_client eh(http_client, "localhost:8080", "", "", "", 1, 1, nullptr, nullptr);

  api_status ret;
  //send events
  BOOST_CHECK_EQUAL(eh.send("message 1", &ret), error_code::success);
  BOOST_CHECK_EQUAL(eh.send("message 2", &ret), error_code::success);
}

BOOST_AUTO_TEST_CASE(retry_http_send_success)
{
  mock_http_client* http_client = new mock_http_client("localhost:8080");

  int tries = 0;
  int succeed_after_n_tries = 3;
  http_client->set_responder(methods::POST, [&tries, succeed_after_n_tries](const http_request& message, http_response& resp) {
    tries++;

    if (tries > succeed_after_n_tries) {
      resp.set_status_code(status_codes::Created);
    }
    else {
      resp.set_status_code(status_codes::InternalError);
    }
  });

  error_counter counter;
  error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    eventhub_client eh(http_client, "localhost:8080", "", "", "", 1, 8 /* retries */, nullptr, &error_callback);

    api_status ret;
    BOOST_CHECK_EQUAL(eh.send("message 1", &ret), error_code::success);
  }

  // Although it was allowed to retry 8 times, it should stop after succeeding at 4.
  BOOST_CHECK_EQUAL(tries, succeed_after_n_tries + 1);
  BOOST_CHECK_EQUAL(counter._err_count, 0);
}

BOOST_AUTO_TEST_CASE(retry_http_send_fail)
{
  mock_http_client* http_client = new mock_http_client("localhost:8080");

  const int MAX_RETRIES = 10;

  int tries = 0;
  http_client->set_responder(methods::POST, [&tries](const http_request& message, http_response& resp) {
    tries++;
    resp.set_status_code(status_codes::InternalError);
  });

  error_counter counter;
  error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    eventhub_client eh(http_client, "localhost:8080", "", "", "", 1, MAX_RETRIES, nullptr, &error_callback);

    api_status ret;
    BOOST_CHECK_EQUAL(eh.send("message 1", &ret), error_code::success);
  }

  BOOST_CHECK_EQUAL(tries, MAX_RETRIES + 1);
  BOOST_CHECK_EQUAL(counter._err_count, 1);
}


BOOST_AUTO_TEST_CASE(http_in_order_after_retry)
{
  mock_http_client* http_client = new mock_http_client("localhost:8080");

  const int MAX_RETRIES = 10;
  int tries = 0;
  std::vector<std::string> received_messages;
  http_client->set_responder(methods::POST, [&tries, &received_messages](const http_request& message, http_response& resp) {
    tries++;

    // Succeed every 4th attempt.
    if (tries >= 4) {
      // extract_string can only be called once on an http_request but we only do it once. Using const cast to avoid having to read out the stream.
      received_messages.push_back(const_cast<http_request&>(message).extract_utf8string().get());
      resp.set_status_code(status_codes::Created);
      tries = 0;
    }
    else {
      resp.set_status_code(status_codes::InternalError);
    }
  });

  error_counter counter;
  error_callback_fn error_callback(&error_counter_func, &counter);

  // Use scope to force destructor and therefore flushing of buffers.
  {
    //create a client
    eventhub_client eh(http_client, "localhost:8080", "", "", "", 1, MAX_RETRIES, nullptr, &error_callback);

    api_status ret;
    BOOST_CHECK_EQUAL(eh.send("message 1", &ret), error_code::success);
    BOOST_CHECK_EQUAL(eh.send("message 2", &ret), error_code::success);
    BOOST_CHECK_EQUAL(eh.send("message 3", &ret), error_code::success);
    BOOST_CHECK_EQUAL(eh.send("message 4", &ret), error_code::success);
    BOOST_CHECK_EQUAL(eh.send("message 5", &ret), error_code::success);
  }

  BOOST_CHECK_EQUAL(received_messages[0], "message 1");
  BOOST_CHECK_EQUAL(received_messages[1], "message 2");
  BOOST_CHECK_EQUAL(received_messages[2], "message 3");
  BOOST_CHECK_EQUAL(received_messages[3], "message 4");
  BOOST_CHECK_EQUAL(received_messages[4], "message 5");
  BOOST_CHECK_EQUAL(counter._err_count, 0);
}
