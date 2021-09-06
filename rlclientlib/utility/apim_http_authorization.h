#pragma once

#include "api_status.h"
#include "authorization.h"

namespace reinforcement_learning {
  class apim_http_authorization : public i_authorization {
  public:
    apim_http_authorization(const std::string& api_key);
    ~apim_http_authorization() = default;

    virtual int init(api_status* status) override;
    virtual int get_http_headers(http_headers& headers, api_status* status) override;

  private:
    apim_http_authorization(const apim_http_authorization&) = delete;
    apim_http_authorization(apim_http_authorization&&) = delete;
    apim_http_authorization& operator=(const apim_http_authorization&) = delete;
    apim_http_authorization& operator=(apim_http_authorization&&) = delete;

  private:
    const std::string _api_key;
  };
}
