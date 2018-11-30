#include "http_client.h"
#include "utility/http_helper.h"

#include <chrono>

namespace reinforcement_learning {
  http_client::http_client(const char* url)
    : _url(url)
    , _impl(::utility::conversions::to_string_t(url), utility::get_http_config()) {
  }

  http_client::response_t http_client::request(http_client::method_t method) {
    return _impl.request(method);
  }

  http_client::response_t http_client::request(http_client::request_t request) {
    return _impl.request(request);
  }

  const std::string& http_client::get_url() const {
    return _url;
  }
}
