#include "eventhub_client.h"

#include "api_status.h"
#include "err_constants.h"
#include "str_util.h"
#include "trace_logger.h"
#include "utility/http_authorization.h"
#include "utility/http_client.h"
#include "utility/stl_container_adapter.h"

namespace u = reinforcement_learning::utility;

namespace reinforcement_learning {

  eventhub_client::http_request_task::http_request_task(
    i_http_client* client,
    const std::string& host,
    const std::string& auth,
    const buffer& post_data,
    size_t max_retries,
    error_callback_fn* error_callback,
    i_trace* trace)
    : _client(client),
    _host(host),
    _auth(auth),
    _post_data(post_data),
    _max_retries(max_retries),
    _error_callback(error_callback),
    _trace(trace)
    {
      _task = send_request(0 /* inital try */);
    }

  std::future<http_response::status_t>
  eventhub_client::http_request_task::send_request(size_t try_count)
  {
    // TODO: Use a custom thread pool to avoid the overhead of std::async thread creation.
    return std::async(std::launch::async, [this, try_count]() {
      http_request request(http_method::POST);
      request.set_header_field("Authorization", _auth.c_str());
      request.set_header_field("Host", _host.c_str());
      request.set_body(_post_data);

      http_response::status_t code = http_response::status::INTERNAL_ERROR;
      api_status status;

      try
      {
        const auto response = _client->request(request);
        code = response->status_code();
      }
      catch (const std::exception &e)
      {
        TRACE_ERROR(_trace, e.what());
      }

      // If the response is not the expected code then it has failed. Retry if possible otherwise report background error.
      if (code != http_response::status::CREATED)
      {
        // TODO: Handle retrying as part of HTTP interface, to avoid repeated reading of the body _post_data.

        // Stop condition of recurison.
        if(try_count < _max_retries){
          TRACE_ERROR(_trace, "HTTP request failed, retrying...");

          // Yes, recursively send another request inside this one. If a subsequent request returns success we are good, otherwise the failure will propagate.
          return send_request(try_count + 1).get();
        }

        auto msg = u::concat("(expected 201): Found ", code, ", failed after ", try_count, " retries.");
        api_status::try_update(&status, error_code::http_bad_status_code, msg.c_str());
        ERROR_CALLBACK(_error_callback, status);
      }

      return code;
    });
  }

  int eventhub_client::http_request_task::join() {
    _task.get();

    // The task may have failed but was reported with the callback. This function's primary purpose
    // is to block if the task is not yet complete.
    return error_code::success;
  }

  int eventhub_client::init(api_status* status) {
    RETURN_IF_FAIL(_authorization.init(_client.get(), status));
    return error_code::success;
  }

  int eventhub_client::pop_task(api_status* status) {
    // This function must be under a lock as there is a delay between popping from the queue and joining the task, but it should essentially be atomic.
    std::lock_guard<std::mutex> lock(_mutex);

    std::unique_ptr<http_request_task> oldest;
    _tasks.pop(&oldest);

    try {
      // This will block if the task is not complete yet.
      RETURN_IF_FAIL(oldest->join());
    }
    catch (...) {
      // Ignore if there is an exception surfaced as this should have been handled in the continuation.
      TRACE_WARN(_trace, "There should not be an exception raised in pop_task function.");
    }

    return error_code::success;
  }

  int eventhub_client::v_send(const buffer& post_data, api_status* status) {

    std::string auth_str;
    RETURN_IF_FAIL(_authorization.get(_client.get(), auth_str, status));

    try {
      // Before creating the task, ensure that it is allowed to be created.
      if (_tasks.size() >= _max_tasks_count) {
        RETURN_IF_FAIL(pop_task(status));
      }

      std::unique_ptr<http_request_task> request_task(new http_request_task(_client.get(), _eventhub_host, auth_str, post_data, _max_retries, _error_callback, _trace));
      _tasks.push(std::move(request_task));
    }
    catch (const std::exception& e) {
      RETURN_ERROR_LS(_trace, status, eventhub_http_generic) << e.what();
    }
    return error_code::success;
  }

  eventhub_client::eventhub_client(i_http_client* client, const std::string& host, const std::string& key_name,
                                   const std::string& key, const std::string& name,
                                   size_t max_tasks_count, size_t max_retries,  i_trace* trace,
                                   error_callback_fn* error_callback)
    : _client(client)
    , _authorization(host, key_name, key, name, trace)
    , _eventhub_host(host)
    , _max_tasks_count(max_tasks_count)
    , _max_retries(max_retries)
    , _trace(trace)
    , _error_callback(error_callback) {
  }

  eventhub_client::~eventhub_client() {
    while (_tasks.size() != 0) {
      pop_task(nullptr);
    }
  }

} // namespace reinforcement_learning
