#include "cpprest_http_client.h"

#include <cpprest/rawptrstream.h>

#include "api_status.h"
#include "constants.h"
#include "stl_container_adapter.h"

namespace reinforcement_learning {

const std::map<http_method, web::http::method>
    cpprest_http_client::_method_map = {
        {http_method::HEAD, web::http::methods::HEAD},
        {http_method::GET, web::http::methods::GET},
        {http_method::POST, web::http::methods::POST}};

web::http::client::http_client_config cpprest_http_client::get_http_config(
    const utility::configuration &cfg) {
  web::http::client::http_client_config config;

  // The default is to validate certificates.
  config.set_validate_certificates(
      !cfg.get_bool(name::HTTP_CLIENT_DISABLE_CERT_VALIDATION, false));
  auto timeout = cfg.get_int(name::HTTP_CLIENT_TIMEOUT, 30);
  // Valid values are 1-30.
  if (timeout < 1 || timeout > 30) timeout = 30;
  config.set_timeout(std::chrono::seconds(timeout));
  return config;
}

int cpprest_http_client::request(const http_request &request,
                                 std::unique_ptr<http_response> &response,
                                 api_status *status, i_trace *trace) {
  pplx::task<web::http::http_response> response_task;

  if (request.method() == http_method::POST) {
    web::http::http_request cpprest_request(web::http::methods::POST);
    for (const auto &field : request.header_fields())
      cpprest_request.headers().add(field.first.c_str(), field.second);

    utility::stl_container_adapter container(request.body().get());
    const size_t container_size = container.size();
    const auto stream =
        concurrency::streams::bytestream::open_istream(container);
    cpprest_request.set_body(stream, container_size);
    response_task = _impl.request(cpprest_request);
  } else {
    // HEAD or GET
    response_task = _impl.request(get_cpprest_method(request.method()));
  }

  try {
    response = std::unique_ptr<http_response>(
        new cpprest_http_response(response_task.get()));
  } catch (const std::exception &e) {
    RETURN_ERROR_LS(trace, status, exception_during_http_req)
        << e.what() << "\n URL: " << get_url();
  }

  return error_code::success;
}

int cpprest_http_response::last_modified(std::string &last_modified_str) const {
  const auto iter = _response.headers().find(U("Last-Modified"));
  if (iter == _response.headers().end())
    return error_code::last_modified_not_found;

  const auto last_modified = ::utility::datetime::from_string(iter->second);
  if (last_modified.to_interval() == 0)
    return error_code::last_modified_invalid;

  last_modified_str =
      ::utility::conversions::to_utf8string(last_modified.to_string());

  return error_code::success;
}

int cpprest_http_response::body(size_t &sz, char *buffer) const {
  try {
    const Concurrency::streams::rawptr_buffer<char> rb(buffer, content_length(),
                                                       std::ios::out);
    sz = _response.body().read_to_end(rb).get();
  } catch (...) {
    return error_code::http_response_read_error;
  }
  return error_code::success;
}

}  // namespace reinforcement_learning
