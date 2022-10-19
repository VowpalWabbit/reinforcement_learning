#pragma once

#include "api_status.h"
#include "configuration.h"
#include "constants.h"

#include <cpprest/http_headers.h>

using namespace web::http;

namespace reinforcement_learning {
  class header_authorization {
  public:
    header_authorization() = default;
    ~header_authorization() = default;

    int init(const utility::configuration& config, api_status* status, i_trace* trace);
    int insert_authorization_header(http_headers& headers, api_status* status, i_trace* trace);

    header_authorization(const header_authorization&) = delete;
    header_authorization(header_authorization&&) = delete;
    header_authorization& operator=(const header_authorization&) = delete;
    header_authorization& operator=(header_authorization&&) = delete;

  private:
    std::string _api_key;
    http_headers::key_type _http_api_header_key_name;
  };
}
