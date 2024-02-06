#pragma once

#include "api_status.h"
#include "configuration.h"
#include "constants.h"
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
class eventhub_headers
{
public:
  void insert_additional_headers(http_headers& headers)
  {
    http_headers::key_type content_type;
#ifdef _WIN32
    content_type = ::utility::conversions::utf8_to_utf16("Content-Type");
#else
    content_type = "Content-Type";
#endif

    headers.add(content_type, "application/atom+xml;type=entry;charset=utf-8");
  }
};
class blob_storage_headers
{
public:
  void insert_additional_headers(http_headers& headers)
  {
    http_headers::key_type version;
#ifdef _WIN32
    version = ::utility::conversions::utf8_to_utf16("x-ms-version");
#else
    version = "x-ms-version";
#endif

    // For version, see https://learn.microsoft.com/en-us/rest/api/storageservices/authorize-with-azure-active-directory
    headers.add(version, "2017-11-09");
  }
};

template <typename Resource>
class api_header_token_callback
{
public:
  api_header_token_callback(oauth_callback_t& token_cb, std::string& scope)
      : _token_callback(token_cb), _scopes{std::move(scope)}
  {
  }
  ~api_header_token_callback() = default;

  int init(const utility::configuration& config, api_status* status, i_trace* trace)
  {
    // The transport client calls init and insert_header on every message
    // Just bail out if we've already been initialized
    if (_initialized) { return error_code::success; }

#ifdef _WIN32
    _http_api_header_key_name = ::utility::conversions::utf8_to_utf16(
        config.get(name::HTTP_API_HEADER_KEY_NAME, value::HTTP_API_DEFAULT_HEADER_KEY_NAME));
#else
    _http_api_header_key_name = config.get(name::HTTP_API_HEADER_KEY_NAME, value::HTTP_API_DEFAULT_HEADER_KEY_NAME);
#endif
    _token_type = config.get(name::HTTP_API_OAUTH_TOKEN_TYPE, value::HTTP_API_DEFAULT_OAUTH_TOKEN_TYPE);
    RETURN_IF_FAIL(refresh_auth_token(status, trace));
    _initialized = true;
    return error_code::success;
  }

  int insert_authorization_header(http_headers& headers, api_status* status, i_trace* trace)
  {
    if (!_initialized)
    {
      int result = error_code::not_initialized;
      api_status::try_update(status, result, error_code::not_initialized_s);
      return result;
    }
    using namespace std::chrono;
    system_clock::time_point now = system_clock::now();
    // TODO: make this configurable?
    system_clock::time_point refresh_time = _token_expiry - std::chrono::seconds(10);
    if (now >= refresh_time) { RETURN_IF_FAIL(refresh_auth_token(status, trace)); }
    std::string header_value = _token_type + " " + _bearer_token;
    headers.add(_http_api_header_key_name, header_value.c_str());
    _additional_headers.insert_additional_headers(headers);

    return error_code::success;
  }

  api_header_token_callback(const api_header_token_callback&) = delete;
  api_header_token_callback(api_header_token_callback&&) = delete;
  api_header_token_callback& operator=(const api_header_token_callback&) = delete;
  api_header_token_callback& operator=(api_header_token_callback&&) = delete;

private:
  int refresh_auth_token(api_status* status, i_trace* trace)
  {
    using namespace std::chrono;
    system_clock::time_point tp;
    RETURN_IF_FAIL(_token_callback(_scopes, _bearer_token, _token_expiry));

    if (_bearer_token.empty())
    {
      int result = error_code::external_error;
      api_status::try_update(status, result, error_code::external_error_s);
      return result;
    }

    if (_bearer_token.empty()) { RETURN_ERROR(trace, status, http_api_key_not_provided); }
    return error_code::success;
  }

private:
  http_headers::key_type _http_api_header_key_name;
  std::string _token_type;
  oauth_callback_t _token_callback;
  std::vector<std::string> _scopes;

  std::string _bearer_token;
  std::chrono::system_clock::time_point _token_expiry;
  bool _initialized = false;
  Resource _additional_headers;
};
}  // namespace reinforcement_learning
