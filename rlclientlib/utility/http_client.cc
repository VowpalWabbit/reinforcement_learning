#include "http_client.h"
#include "utility/http_helper.h"

#include <chrono>

namespace reinforcement_learning {
  class http_client : public i_http_client {
  public:
    http_client(const char* url, const utility::configuration& cfg)
    : _url(url)
    , _impl(::utility::conversions::to_string_t(url), utility::get_http_config(cfg)) {
    }


    http_client(http_client&& other) = delete;
    http_client& operator=(http_client&& other) = delete;
    http_client(const http_client&) = delete;
    http_client& operator=(const http_client&) = delete;

    virtual response_t request(method_t method) override {
      return _impl.request(method);
    }

    virtual response_t request(request_t request) override {
      return _impl.request(request);
    }

    virtual const std::string& get_url() const override {
      return _url;
    }

  private:
    const std::string _url;
    web::http::client::http_client _impl;
  };

  int create_http_client(const char* url, const utility::configuration& cfg, i_http_client** client, api_status* status) {
    int result = error_code::success;
    try {
      *client = new http_client(url, cfg);
    } catch (const std::exception& e) {
      result = error_code::http_client_init_error;
      api_status::try_update(status, result, e.what());
    } catch (...) {
      result = error_code::http_client_init_error;
      api_status::try_update(status, result, error_code::http_client_init_error_s);
    }
    return result;
  }
}
