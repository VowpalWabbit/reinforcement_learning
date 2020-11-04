#pragma once

#include <memory>

#include "model_mgmt.h"
#include "utility/http_client.h"

namespace reinforcement_learning
{

  class i_trace;

  namespace model_management
  {

    // TODO: Add tests for restapi_data_transport.

    class restapi_data_transport : public i_data_transport
    {
    public:
      // Takes the ownership of the i_http_client and delete it at the end of lifetime
      restapi_data_transport(i_http_client *httpcli, i_trace *trace);

      int get_data(model_data &data, api_status *status) override;

    private:
      int get_data_info(std::string &last_modified, uint64_t &sz, api_status *status);

      std::unique_ptr<i_http_client> _httpcli;
      std::string _last_modified;
      uint64_t _datasz;
      i_trace *_trace;
    };

  } // namespace model_management
} // namespace reinforcement_learning
