#include "header_authorization.h"
#include <boost/algorithm/string.hpp>
#include <codecvt>

using namespace utility;
using convert_t = std::codecvt_utf8<wchar_t>;

namespace reinforcement_learning {
  int header_authorization::init(const utility::configuration& config, api_status* status, i_trace* trace) {
    const auto api_key = config.get(name::HTTP_API_KEY, nullptr);
    if (api_key == nullptr) {
      RETURN_ERROR(trace, status, http_api_key_not_provided);
    }
    _api_key = api_key;
    std::wstring_convert<convert_t, wchar_t> strconverter;
    _header_L_name = strconverter.from_bytes(config.get(value::HEADER_NAME, value::DEFAULT_HEADER));
    return error_code::success;
  }

  int header_authorization::get_http_headers(http_headers& headers, api_status* status) {
    const utility::configuration config;
    headers.add(_header_L_name, _api_key.c_str()); 
    return error_code::success;
  }
}
