#include "header_authorization.h"
#include <boost/algorithm/string.hpp>

using namespace utility;

namespace reinforcement_learning {
  std::string enum_to_string(authenticatinType type) {
    switch (type) {
      case authenticatinType::BearerToken:
        return "bearertoken";
      case authenticatinType::ApiKey:
        return "apikey";
    }
  }
  int header_authorization::init(const utility::configuration& config, api_status* status, i_trace* trace) {
    const auto api_key = config.get(name::HTTP_API_KEY, nullptr);
    if (api_key == nullptr) {
      RETURN_ERROR(trace, status, http_api_key_not_provided);
    }
    const auto auth_type = config.get(name::HTTP_KEY_TYPE, nullptr);
    if (auth_type == nullptr || !(boost::iequals(auth_type, enum_to_string(authenticatinType::BearerToken)) || boost::iequals(auth_type, enum_to_string(authenticatinType::ApiKey))) ) {
        RETURN_ERROR(trace, status, http_auth_type_not_provided);
    }

    _api_key = api_key;
    _auth_type = auth_type;

    return error_code::success;
  }

  int header_authorization::get_http_headers(http_headers& headers, api_status* status) {
      const utility::configuration config;
      if (boost::iequals(_auth_type, enum_to_string(authenticatinType::BearerToken)))
      {
          std::string bearerToken = config.get(value::BEARER, "Bearer ") + _api_key;
          headers.add(_XPLATSTR("Authorization"), bearerToken.c_str());
      }
      else
      {
          headers.add(_XPLATSTR("Ocp - Apim - Subscription - Key"), _api_key.c_str());
      }
    return error_code::success;
  }
}
