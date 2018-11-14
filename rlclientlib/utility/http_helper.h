#pragma once
#include <cpprest/http_client.h>

namespace reinforcement_learning {
  namespace utility {

    web::http::client::http_client_config get_http_config();

    class i_http_client {
    public:
      using request_t = typename web::http::http_request;
      using task_t = typename pplx::task<web::http::http_response>;
      typedef web::http::method method_t;

    public:
      virtual task_t request(method_t) = 0;
      virtual task_t request(request_t) = 0;

      virtual const std::string& get_url() const = 0;
    };

    class http_client : public i_http_client {
    public:
      http_client(const char* url);

      http_client(http_client&& other) = delete;
      http_client& operator=(http_client&& other) = delete;
      http_client(const http_client&) = delete;
      http_client& operator=(const http_client&) = delete;

      virtual task_t request(method_t method) override;
      virtual task_t request(request_t method) override;
      virtual const std::string& get_url() const override;

    private:
      std::string _url;
      web::http::client::http_client _impl;
    };
  }
}