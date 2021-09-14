#pragma once

#include "apim_http_authorization.h"

namespace reinforcement_learning {
  int apim_http_authorization::init(const utility::configuration& config, api_status* status, i_trace* trace) {
    _api_key = config.get(name::HTTP_API_KEY, "dummykey");
    return error_code::success;
  }

  int apim_http_authorization::get_http_headers(http_headers& headers, api_status* status) {
    headers.add(_XPLATSTR("Ocp-Apim-Subscription-Key"), _api_key.c_str());
    return error_code::success;
  }
}
