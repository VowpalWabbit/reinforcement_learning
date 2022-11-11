#pragma once

#include "api_status.h"
#include "data_buffer.h"
#include "err_constants.h"
#include "error_callback_fn.h"
#include "moving_queue.h"
#include "sender.h"
#include "str_util.h"
#include "trace_logger.h"
#include "utility/eventhub_http_authorization.h"
#include "utility/header_authorization.h"
#include "utility/http_client.h"
#include "utility/stl_container_adapter.h"

#include <cpprest/http_headers.h>
#include <pplx/pplxtasks.h>

#include <memory>
#include <sstream>

using namespace web::http;
using namespace std::chrono;
using namespace utility;  // Common utilities like string conversions
using namespace web;      // Common features like URIs.
namespace u = reinforcement_learning::utility;

namespace reinforcement_learning
{
class i_trace;

template <typename TAuthorization>
// The http_transport_client sends string data in POST requests to an HTTP endpoint using the passed in authorization
// headers.
class http_transport_client : public i_sender
{
public:
  virtual int init(const utility::configuration& config, api_status* status) override;

  // Takes the ownership of the i_http_client and delete it at the end of lifetime
  http_transport_client(
      i_http_client* client, size_t tasks_count, size_t MAX_RETRIES, i_trace* trace, error_callback_fn* _error_cb);
  ~http_transport_client();

protected:
  int v_send(const buffer& data, api_status* status) override;

private:
  class http_request_task
  {
  public:
    using buffer = std::shared_ptr<utility::data_buffer>;
    http_request_task() = default;
    http_request_task(i_http_client* client, http_headers headers, const buffer& data,
        size_t max_retries = 1,  // If MAX_RETRIES is set to 1, only the initial request will be attempted.
        error_callback_fn* error_callback = nullptr, i_trace* trace = nullptr);

    // The constructor kicks off an async request which captures the this variable. If this object is moved then the
    // this pointer is invalidated and causes tricky bugs.
    http_request_task(http_request_task&& other) = delete;
    http_request_task& operator=(http_request_task&& other) = delete;
    http_request_task(const http_request_task&) = delete;
    http_request_task& operator=(const http_request_task&) = delete;

    // Return error_code
    int join();

  private:
    // repeat request until success or _max_retries attempted
    // returns the http::status_code of the final attempt.
    pplx::task<web::http::status_code> send_request();

    // repeat request until success or _max_retries attempted
    // returns the http_reponse of the final attempt.
    pplx::task<http_response> send_request_with_retries(size_t try_count);

    i_http_client* _client;
    http_headers _headers;
    buffer _post_data;

    pplx::task<web::http::status_code> _task;

    size_t _max_retries = 1;

    error_callback_fn* _error_callback;
    i_trace* _trace;
  };

private:
  int pop_task(api_status* status);

  // cannot be copied or assigned
  http_transport_client(const http_transport_client&) = delete;
  http_transport_client(http_transport_client&&) = delete;
  http_transport_client& operator=(const http_transport_client&) = delete;
  http_transport_client& operator=(http_transport_client&&) = delete;

private:
  std::unique_ptr<i_http_client> _client;
  TAuthorization _authorization;

  std::mutex _mutex;
  moving_queue<std::unique_ptr<http_request_task>> _tasks;
  const size_t _max_tasks_count;
  const size_t _max_retries;
  i_trace* _trace;
  error_callback_fn* _error_callback;
};

template <typename TAuthorization>
http_transport_client<TAuthorization>::http_request_task::http_request_task(i_http_client* client, http_headers headers,
    const buffer& post_data, size_t max_retries, error_callback_fn* error_callback, i_trace* trace)
    : _client(client)
    , _headers(headers)
    , _post_data(post_data)
    , _max_retries(max_retries)
    , _error_callback(error_callback)
    , _trace(trace)
{
  _task = send_request();
}

template <typename TAuthorization>
pplx::task<web::http::status_code> http_transport_client<TAuthorization>::http_request_task::send_request()
{
  return send_request_with_retries(0 /* inital try */).then([this](pplx::task<http_response> response)
    {
      web::http::status_code code = status_codes::InternalError;

      try
      {
        code = response.get().status_code();
      }
      catch (const std::exception& e)
      {
        TRACE_ERROR(_trace, e.what());
      }

      return code;
    }
  );
}


template <typename TAuthorization>
pplx::task<http_response> http_transport_client<TAuthorization>::http_request_task::send_request_with_retries(
    size_t try_count)
{
  http_request request(methods::POST);
  request.headers() = _headers;

  utility::stl_container_adapter container(_post_data.get());
  const size_t container_size = container.size();
  const auto stream = concurrency::streams::bytestream::open_istream(container);
  request.set_body(stream, container_size);

  // lambda which examines the provided task and either 1) generates a replacement task to retry
  // the request, or 2) passes the provided task downstream to emit its response
  auto retry_request_on_failure_lambda = [this, try_count](pplx::task<http_response> response_task) -> pplx::task<http_response>
  {
    web::http::status_code response_code = status_codes::InternalError;
    
    try
    {
      response_code = response_task.get().status_code();
    }
    catch (const std::exception& e)
    {
      TRACE_ERROR(_trace, e.what());
    }

    bool success = (response_code >= status_codes::OK) && (response_code < status_codes::MultipleChoices);
    if (success)
    {
      return response_task;
    }

    // If the response is not success class then it has failed. Retry if possible otherwise report background error.

    if (try_count >= _max_retries)
    {
      // We have exhausted retry attempts, log and return the task describing the failure
      api_status status;
      auto msg = u::concat("(expected 201): Found ", response_code, ", failed after ", try_count, " retries.");
      api_status::try_update(&status, error_code::http_bad_status_code, msg.c_str());
      ERROR_CALLBACK(_error_callback, status);

      return response_task;
    }

    if (status_codes::TooManyRequests == response_code)
    {
      // if server says it's too busy, delay retry for a while to give it time to recover
      static int const LINEAR_DELAY_MS_PER_FAILURE = 2000;
      int delay_ms = (1 + try_count) * LINEAR_DELAY_MS_PER_FAILURE;

      TRACE_ERROR(_trace, u::concat("HTTP request failed with ", response_code, ", retrying in ", delay_ms, "ms..."));      

      pplx::wait(delay_ms);
    }
    else
    {
      TRACE_ERROR(_trace, u::concat("HTTP request failed with ", response_code, ", retrying..."));
    }

    // return a new task which will resubmit the original request
    return send_request_with_retries(try_count + 1);
  };

  return _client->request(request).then(retry_request_on_failure_lambda);
}

template <typename TAuthorization>
int http_transport_client<TAuthorization>::http_request_task::join()
{
  _task.get();

  // The task may have failed but was reported with the callback. This function's primary purpose
  // is to block if the task is not yet complete.
  return error_code::success;
}

template <typename TAuthorization>
int http_transport_client<TAuthorization>::init(const utility::configuration& config, api_status* status)
{
  RETURN_IF_FAIL(_authorization.init(config, status, _trace));
  return error_code::success;
}

template <typename TAuthorization>
int http_transport_client<TAuthorization>::pop_task(api_status* status)
{
  // This function must be under a lock as there is a delay between popping from the queue and joining the task, but it
  // should essentially be atomic.
  std::lock_guard<std::mutex> lock(_mutex);

  std::unique_ptr<http_request_task> oldest;
  _tasks.pop(&oldest);

  try
  {
    // This will block if the task is not complete yet.
    RETURN_IF_FAIL(oldest->join());
  }
  catch (...)
  {
    // Ignore if there is an exception surfaced as this should have been handled in the continuation.
    TRACE_WARN(_trace, "There should not be an exception raised in pop_task function.");
  }

  return error_code::success;
}

template <typename TAuthorization>
int http_transport_client<TAuthorization>::v_send(const buffer& post_data, api_status* status)
{
  http_headers headers;
  RETURN_IF_FAIL(_authorization.insert_authorization_header(headers, status, _trace));

  try
  {
    // Before creating the task, ensure that it is allowed to be created.
    if (_tasks.size() >= _max_tasks_count) { RETURN_IF_FAIL(pop_task(status)); }

    std::unique_ptr<http_request_task> request_task(
        new http_request_task(_client.get(), headers, post_data, _max_retries, _error_callback, _trace));
    _tasks.push(std::move(request_task));
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_LS(_trace, status, eventhub_http_generic) << e.what();
  }
  return error_code::success;
}

template <typename TAuthorization>
http_transport_client<TAuthorization>::http_transport_client(i_http_client* client, size_t max_tasks_count,
    size_t max_retries, i_trace* trace, error_callback_fn* error_callback)
    : _client(client)
    , _max_tasks_count(max_tasks_count)
    , _max_retries(max_retries)
    , _trace(trace)
    , _error_callback(error_callback)
{
}

template <typename TAuthorization>
http_transport_client<TAuthorization>::~http_transport_client()
{
  while (_tasks.size() != 0)
  {
    auto result = pop_task(nullptr);
    if (!result)
    {
      auto message =
          utility::concat("Failure in pop_task while running ~http_transport_client. Status: '", result, "'");
      TRACE_ERROR(_trace, message);
    }
  }
}
}  // namespace reinforcement_learning
