#include "header_authorization.h"

namespace reinforcement_learning
{
int header_authorization::init(const utility::configuration& config, api_status* status, i_trace* trace)
{
  const auto* api_key = config.get(name::HTTP_API_KEY, nullptr);
  if (api_key == nullptr) { RETURN_ERROR(trace, status, http_api_key_not_provided); }
  _api_key = api_key;
#ifdef _WIN32
  _http_api_header_key_name = ::utility::conversions::utf8_to_utf16(
      config.get(name::HTTP_API_HEADER_KEY_NAME, value::HTTP_API_DEFAULT_HEADER_KEY_NAME));
#else
  _http_api_header_key_name = config.get(name::HTTP_API_HEADER_KEY_NAME, value::HTTP_API_DEFAULT_HEADER_KEY_NAME);
#endif
  return error_code::success;
}

int header_authorization::insert_authorization_header(http_headers& headers, api_status* status, i_trace* trace)
{
  if (_api_key.empty()) { RETURN_ERROR(trace, status, http_api_key_not_provided); }
  headers.add(_http_api_header_key_name, _api_key.c_str());
  return error_code::success;
}
}  // namespace reinforcement_learning
