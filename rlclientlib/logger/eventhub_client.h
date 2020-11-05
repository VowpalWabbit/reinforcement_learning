#pragma once

#include <future>
#include <memory>

#include "api_status.h"
#include "data_buffer.h"
#include "error_callback_fn.h"
#include "moving_queue.h"
#include "sender.h"
#include "utility/http_authorization.h"
#include "utility/http_client.h"

namespace reinforcement_learning {

class i_trace;

// The eventhub_client send string data in POST requests to an HTTP endpoint.
// It handles authorization headers specific for the Azure event hubs.
class eventhub_client : public i_sender {
 public:
  virtual int init(api_status* status) override;

  eventhub_client(std::unique_ptr<i_http_client> client,
                  const std::string& host, const std::string& key_name,
                  const std::string& key, const std::string& name,
                  size_t tasks_count, size_t MAX_RETRIES, i_trace* trace,
                  error_callback_fn* _error_cb);
  ~eventhub_client();

 protected:
  int v_send(const buffer& data, api_status* status) override;

 private:
  class http_request_task {
   public:
    using buffer = std::shared_ptr<utility::data_buffer>;

    http_request_task() = default;
    http_request_task(
        i_http_client* client, const std::string& host, const std::string& auth,
        const buffer& data,
        size_t max_retries = 1,  // If MAX_RETRIES is set to 1, only the initial
                                 // request will be attempted.
        error_callback_fn* error_callback = nullptr, i_trace* trace = nullptr);

    // The constructor kicks off an async request which captures the this
    // variable. If this object is moved then the this pointer is invalidated
    // and causes tricky bugs.
    http_request_task(http_request_task&& other) = delete;
    http_request_task& operator=(http_request_task&& other) = delete;
    http_request_task(const http_request_task&) = delete;
    http_request_task& operator=(const http_request_task&) = delete;

    // Returns error_code.
    int join();

   private:
    std::future<http_response::status_t> send_request(size_t try_count);

    i_http_client* _client;
    std::string _host;
    std::string _auth;
    buffer _post_data;

    std::future<http_response::status_t> _task;

    size_t _max_retries = 1;

    error_callback_fn* _error_callback;
    i_trace* _trace;
  };

 private:
  int pop_task(api_status* status);

  // cannot be copied or assigned
  eventhub_client(const eventhub_client&) = delete;
  eventhub_client(eventhub_client&&) = delete;
  eventhub_client& operator=(const eventhub_client&) = delete;
  eventhub_client& operator=(eventhub_client&&) = delete;

 private:
  std::unique_ptr<i_http_client> _client;
  http_authorization _authorization;
  const std::string
      _eventhub_host;  // e.g. "ingest-x2bw4dlnkv63q.servicebus.windows.net"

  std::mutex _mutex;
  moving_queue<std::unique_ptr<http_request_task>> _tasks;
  const size_t _max_tasks_count;
  const size_t _max_retries;
  i_trace* _trace;
  error_callback_fn* _error_callback;
};

}  // namespace reinforcement_learning
