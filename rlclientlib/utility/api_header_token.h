#pragma once

#include "configuration.h"
#include "oauth_callback_fn.h"
#include "trace_logger.h"

#include <cpprest/http_headers.h>

#include <chrono>
#include <functional>
#include <string>
#include <vector>

using namespace web::http;

namespace reinforcement_learning
{
class api_header_token_callback
{
public:
  api_header_token_callback(oauth_callback_t token_cb, std::string scope);
  ~api_header_token_callback() = default;

  int init(const utility::configuration& config, api_status* status, i_trace* trace);

  int insert_authorization_header(http_headers& headers, api_status* status, i_trace* trace);

  api_header_token_callback(const api_header_token_callback&) = delete;
  api_header_token_callback(api_header_token_callback&&) = delete;
  api_header_token_callback& operator=(const api_header_token_callback&) = delete;
  api_header_token_callback& operator=(api_header_token_callback&&) = delete;

private:
  int refresh_auth_token(api_status* status, i_trace* trace);
private:
  http_headers::key_type _http_api_header_key_name;
  std::string _token_type;
  oauth_callback_t _token_callback;
  std::vector<std::string> _scopes;

  std::string _bearer_token;
  std::chrono::system_clock::time_point _token_expiry;
  bool _initialized = false;
};
}  // namespace reinforcement_learning
