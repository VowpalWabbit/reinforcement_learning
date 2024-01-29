#include "api_header_token.h"
#include "api_status.h"
#include "configuration.h"
#include "constants.h"
#include "time_helper.h"

#include <cpprest/http_headers.h>

#include <chrono>
#include <functional>
#include <vector>

#include <iomanip>

using namespace web::http;

namespace reinforcement_learning
{
api_header_token_callback::api_header_token_callback(oauth_callback_t token_cb, std::string scope) :
_token_callback(token_cb),
_scopes{std::move(scope)}
{}

int api_header_token_callback::init(const utility::configuration& config, api_status* status, i_trace* trace)
{
#ifdef _WIN32
  _http_api_header_key_name = ::utility::conversions::utf8_to_utf16(
      config.get(name::HTTP_API_HEADER_KEY_NAME, value::HTTP_API_DEFAULT_HEADER_KEY_NAME));
  _token_type = ::utility::conversions::utf8_to_utf16(
      config.get(name::HTTP_API_OAUTH_TOKEN_TYPE, value::HTTP_API_DEFAULT_OAUTH_TOKEN_TYPE));
#else
  _http_api_header_key_name = config.get(name::HTTP_API_HEADER_KEY_NAME, value::HTTP_API_DEFAULT_HEADER_KEY_NAME);
  _token_type = config.get(name::HTTP_API_OAUTH_TOKEN_TYPE, value::HTTP_API_DEFAULT_OAUTH_TOKEN_TYPE);
#endif
  RETURN_IF_FAIL(refresh_auth_token(status, trace));
  _initialized = true;
  return error_code::success;
}

int api_header_token_callback::insert_authorization_header(http_headers& headers, api_status* status, i_trace* trace)
{
  if (!_initialized)
  {
    int result = error_code::not_initialized;
    api_status::try_update(status, result, error_code::not_initialized_s);
    return result;
  }
  using namespace std::chrono_literals;
  using namespace std::chrono;
  system_clock::time_point now = system_clock::now();
  system_clock::time_point refresh_time = _token_expiry - 10s;
  if (now >= refresh_time)
  {
    RETURN_IF_FAIL(refresh_auth_token(status, trace));
  }
  std::string header_value = _token_type + " " + _bearer_token;
  headers.add(_http_api_header_key_name, header_value.c_str());
  return error_code::success;
}

int api_header_token_callback::refresh_auth_token(api_status* status, i_trace* trace)
{
  using namespace std::chrono_literals;
  using namespace std::chrono;
  system_clock::time_point tp;
  RETURN_IF_FAIL(_token_callback(_bearer_token, _token_expiry, _scopes));

  if (_bearer_token.empty())
  {
    int result = error_code::external_error;
    api_status::try_update(status, result, error_code::external_error_s);
    return result;
  }

  if (_bearer_token.empty()) { RETURN_ERROR(trace, status, http_api_key_not_provided); }
  return error_code::success;
}
}  // namespace reinforcement_learning
