#include "apim_http_authorization.h"
#include <boost/algorithm/string.hpp>

namespace reinforcement_learning {
  int apim_http_authorization::init(const utility::configuration& config, api_status* status, i_trace* trace) {
    const auto api_key = config.get(name::HTTP_API_KEY, nullptr);
    if (api_key == nullptr) {
      RETURN_ERROR(trace, status, http_api_key_not_provided);
    }
    const auto auth_type = config.get(name::HTTP_KEY_TYPE, nullptr);
    if (auth_type == nullptr) {
        RETURN_ERROR(trace, status, http_auth_type_not_provided);
    }

    _api_key = api_key;
    _auth_type = auth_type;

    return error_code::success;
  }

  int apim_http_authorization::get_http_headers(http_headers& headers, api_status* status) {
      if (boost::iequals(_auth_type, "bearer"))
      {
          std::string bearerToken = "Bearer " + _api_key;
          headers.add(_XPLATSTR("Authorization"), bearerToken.c_str());
      }
      else
      {
          headers.add(_XPLATSTR("Ocp-Apim-Subscription-Key"), _api_key.c_str());
      }
    return error_code::success;
  }
}
