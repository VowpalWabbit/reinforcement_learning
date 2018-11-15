#pragma once
#include <cpprest/http_client.h>

namespace reinforcement_learning {
  class i_http_client {
  public:
    typedef web::http::http_request request_t;
    typedef pplx::task<web::http::http_response> response_t;
    typedef web::http::method method_t;

  public:
    virtual ~i_http_client() = default;

    virtual response_t request(method_t) = 0;
    virtual response_t request(request_t) = 0;

    virtual const std::string& get_url() const = 0;
  };

  class http_client : public i_http_client {
  public:
    http_client(const char* url);

    http_client(http_client&& other) = delete;
    http_client& operator=(http_client&& other) = delete;
    http_client(const http_client&) = delete;
    http_client& operator=(const http_client&) = delete;

    virtual response_t request(method_t method) override;
    virtual response_t request(request_t method) override;

    virtual const std::string& get_url() const override;

  private:
    std::string _url;
    web::http::client::http_client _impl;
  };
}