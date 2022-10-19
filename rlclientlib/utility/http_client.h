#pragma once
#include <cpprest/http_client.h>
#include "api_status.h"
#include "configuration.h"

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

  int create_http_client(const char* url, const utility::configuration& cfg, i_http_client** client, api_status* status = nullptr);
}
