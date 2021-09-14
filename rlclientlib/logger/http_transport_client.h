#pragma once

#include "api_status.h"
#include "moving_queue.h"
#include "sender.h"
#include "error_callback_fn.h"

#include "utility/http_client.h"
#include "utility/apim_http_authorization.h"
#include "utility/eventhub_http_authorization.h"

#include <pplx/pplxtasks.h>
#include <cpprest/http_headers.h>

#include <memory>
#include "data_buffer.h"

using namespace web::http;

namespace reinforcement_learning {
  class i_trace;

  template <typename T>
  // The http_transport_client sends string data in POST requests to an HTTP endpoint using the passed in authorization headers.
  class http_transport_client : public i_sender {
  public:
    virtual int init(const utility::configuration& config, api_status* status) override;

    // Takes the ownership of the i_http_client and delete it at the end of lifetime
    http_transport_client(i_http_client* client, size_t tasks_count, size_t MAX_RETRIES, i_trace* trace, error_callback_fn* _error_cb);
    ~http_transport_client();
  protected:
    int v_send(const buffer& data, api_status* status) override;

  private:
    class http_request_task {
    public:
      using buffer = std::shared_ptr< utility::data_buffer>;
      http_request_task() = default;
      http_request_task(
        i_http_client* client,
        http_headers headers,
        const buffer& data,
        size_t max_retries = 1, // If MAX_RETRIES is set to 1, only the initial request will be attempted.
        error_callback_fn* error_callback = nullptr,
        i_trace* trace = nullptr);

      // The constructor kicks off an async request which captures the this variable. If this object is moved then the
      // this pointer is invalidated and causes tricky bugs.
      http_request_task(http_request_task&& other) = delete;
      http_request_task& operator=(http_request_task&& other) = delete;
      http_request_task(const http_request_task&) = delete;
      http_request_task& operator=(const http_request_task&) = delete;

      // Return error_code
      int join();
    private:
      pplx::task<web::http::status_code> send_request(size_t try_count);

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
    T _authorization;

    std::mutex _mutex;
    moving_queue<std::unique_ptr<http_request_task>> _tasks;
    const size_t _max_tasks_count;
    const size_t _max_retries;
    i_trace* _trace;
    error_callback_fn* _error_callback;
  };
}
