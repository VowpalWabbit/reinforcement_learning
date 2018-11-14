#include "http_helper.h"

#include <chrono>

namespace reinforcement_learning {
  namespace utility {
    web::http::client::http_client_config get_http_config() {
      web::http::client::http_client_config config;

      config.set_validate_certificates(false);
      // Set a timeout for network requests to avoid hanging too long because of network issues.
      config.set_timeout(std::chrono::seconds(2));
      return config;
    }

    http_client::http_client(const char* url)
      : _url(url)
      , _impl(::utility::conversions::to_string_t(url), get_http_config()) {
    }

    http_client::task_t http_client::request(http_client::method_t method) {
      return _impl.request(method);
    }

    http_client::task_t http_client::request(http_client::request_t request) {
      return _impl.request(request);
    }

    const std::string& http_client::get_url() const {
      return _url;
    }
  }
}
