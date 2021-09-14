#pragma once

#include "api_status.h"
#include "configuration.h"
#include "constants.h"

#include <cpprest/http_headers.h>

using namespace web::http;

namespace reinforcement_learning {
  class apim_http_authorization {
  public:
    apim_http_authorization() = default;
    ~apim_http_authorization() = default;

    int init(const utility::configuration& config, api_status* status, i_trace* trace);
    int get_http_headers(http_headers& headers, api_status* status);

  private:
    apim_http_authorization(const apim_http_authorization&) = delete;
    apim_http_authorization(apim_http_authorization&&) = delete;
    apim_http_authorization& operator=(const apim_http_authorization&) = delete;
    apim_http_authorization& operator=(apim_http_authorization&&) = delete;

  private:
    std::string _api_key;
  };
}
