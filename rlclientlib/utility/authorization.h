#pragma once
#include <cpprest/http_headers.h>

using namespace web::http;

namespace reinforcement_learning {
  class api_status;
  class i_authorization	{
  public:
    virtual int init(api_status* status) = 0;
    virtual int get_http_headers(http_headers& headers, api_status* status) = 0;
    virtual ~i_authorization() = default;
  };
}
