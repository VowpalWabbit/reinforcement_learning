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
  }
}
