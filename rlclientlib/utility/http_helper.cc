#include "http_helper.h"

#include "constants.h"

#include <chrono>
#include <limits>

namespace reinforcement_learning
{
namespace utility
{
web::http::client::http_client_config get_http_config(const utility::configuration& cfg)
{
  web::http::client::http_client_config config;

  // The default is to validate certificates.
  config.set_validate_certificates(!cfg.get_bool(name::HTTP_CLIENT_DISABLE_CERT_VALIDATION, false));
  auto timeout = cfg.get_int(name::HTTP_CLIENT_TIMEOUT, 30);
  // HTTP backends may represent the timeout as signed integer milliseconds.
  const auto max_timeout_seconds = (std::numeric_limits<int>::max)() / 1000;
  if (timeout < 1 || timeout > max_timeout_seconds) { timeout = 30; }
  config.set_timeout(std::chrono::seconds(timeout));
  return config;
}
}  // namespace utility
}  // namespace reinforcement_learning
