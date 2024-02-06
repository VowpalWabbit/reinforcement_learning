#pragma once
#include "model_mgmt.h"
#include "oauth_callback_fn.h"
#include "utility/api_header_token.h"
#include "utility/http_client.h"

#include <cpprest/http_headers.h>

#include <chrono>
#include <string>

// TODO: This is basically just a copy/paste of restapi_data_transport
// We could templatize that object similar to how http_transport_client works,
// but there's a lot of code that would need to be shifted to the header
namespace reinforcement_learning
{
class i_trace;
namespace model_management
{
class restapi_data_transport_oauth : public i_data_transport
{
public:
  // Takes the ownership of the i_http_client and delete it at the end of lifetime
  restapi_data_transport_oauth(i_http_client* httpcli, i_trace* trace, oauth_callback_t& callback, std::string scope);
  restapi_data_transport_oauth(std::unique_ptr<i_http_client>&& httpcli, utility::configuration cfg,
      model_source model_source, i_trace* trace, oauth_callback_t& callback, std::string scope);

  int get_data(model_data& ret, api_status* status) override;

private:
  using time_t = std::chrono::time_point<std::chrono::system_clock>;
  int get_data_info(::utility::datetime& last_modified, ::utility::size64_t& sz, api_status* status);
  int add_authentiction_header(http_headers& header, api_status* status);
  std::unique_ptr<i_http_client> _httpcli;
  ::utility::datetime _last_modified;
  uint64_t _datasz;
  i_trace* _trace;
  const utility::configuration _cfg;
  model_source _model_source = model_source::AZURE;
  method _method_type = methods::HEAD;
  bool _retry_get_data = true;
  api_header_token_callback<blob_storage_headers> _headerimpl;
};
}  // namespace model_management
}  // namespace reinforcement_learning
