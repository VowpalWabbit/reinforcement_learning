#pragma once
#include "model_mgmt.h"
#include <chrono>
#include <string>
#include "http_client.h"

namespace reinforcement_learning {
  class i_trace;
  namespace model_management {
  class restapi_data_tranport : public i_data_transport {
  public:
    restapi_data_tranport(i_http_client* httpcli, i_trace* trace);
    int get_data(model_data& data, api_status* status) override;
    int check(api_status* status);
  private:
    using time_t = std::chrono::time_point<std::chrono::system_clock>;
    int get_data_info(::utility::datetime& last_modified, ::utility::size64_t& sz, api_status* status);
    std::unique_ptr<i_http_client> _httpcli;
    ::utility::datetime _last_modified;
    uint64_t _datasz;
    i_trace* _trace;
  };
}}
