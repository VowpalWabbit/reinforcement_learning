#pragma once
#include "apim_http_authorization.h"

namespace reinforcement_learning {
  apim_http_authorization::apim_http_authorization(
    const std::string& api_key)
    : _api_key(api_key) {
  }

  int apim_http_authorization::init(api_status* status) {
    return error_code::success;
  }

  int apim_http_authorization::get_http_headers(http_headers& headers, api_status* status) {
    headers.add(_XPLATSTR("Ocp-Apim-Subscription-Key"), _api_key.c_str());
    return error_code::success;
  }
}
