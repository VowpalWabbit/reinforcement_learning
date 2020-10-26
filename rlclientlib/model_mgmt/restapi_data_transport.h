#pragma once
#include "model_mgmt.h"
#include "utility/http_client.h"

#include <chrono>
#include <string>

namespace reinforcement_learning {
  class i_trace;
  namespace model_management {
  class restapi_data_transport : public i_data_transport {
  public:
    // Takes the ownership of the i_http_client and delete it at the end of lifetime
    restapi_data_transport(i_http_client* httpcli, i_trace* trace);

    int get_data(model_data& data, api_status* status) override;
  private:
    using time_t = std::chrono::time_point<std::chrono::system_clock>;
    // TODO: remove the use of ::utility::datetime which is part of cpprestsdk.
    int get_data_info(::utility::datetime& last_modified, ::utility::size64_t& sz, api_status* status);
    std::unique_ptr<i_http_client> _httpcli;
    ::utility::datetime _last_modified;
    uint64_t _datasz;
    i_trace* _trace;
  };
}}
