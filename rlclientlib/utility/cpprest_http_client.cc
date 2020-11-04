#include "cpprest_http_client.h"

#include <cpprest/rawptrstream.h>

#include "api_status.h"
#include "constants.h"
#include "stl_container_adapter.h"

namespace reinforcement_learning
{

  const std::map<http_method, web::http::method> cpprest_http_client::_method_map = {
      {http_method::HEAD, web::http::methods::HEAD},
      {http_method::GET, web::http::methods::GET},
      {http_method::POST, web::http::methods::POST}};

  web::http::client::http_client_config cpprest_http_client::get_http_config(const utility::configuration &cfg)
  {
    web::http::client::http_client_config config;

    // The default is to validate certificates.
    config.set_validate_certificates(!cfg.get_bool(name::HTTP_CLIENT_DISABLE_CERT_VALIDATION, false));
    auto timeout = cfg.get_int(name::HTTP_CLIENT_TIMEOUT, 30);
    // Valid values are 1-30.
    if (timeout < 1 || timeout > 30)
      timeout = 30;
    config.set_timeout(std::chrono::seconds(timeout));
    return config;
  }

  std::unique_ptr<http_response> cpprest_http_client::request(const http_request &request)
  {
    pplx::task<web::http::http_response> response_task;

    if (request.method() == http_method::POST)
    {
      web::http::http_request cpprest_request(web::http::methods::POST);
      for (const auto &p : request._header_fields)
        cpprest_request.headers().add(_XPLATSTR(p.first.c_str()), p.second);

      utility::stl_container_adapter container(request._body_buffer.get());
      const size_t container_size = container.size();
      const auto stream = concurrency::streams::bytestream::open_istream(container);
      cpprest_request.set_body(stream, container_size);
      response_task = _impl.request(cpprest_request);
    }
    else
    {
      // HEAD or GET
      response_task = _impl.request(get_cpprest_method(request.method()));
    }

    try
    {
      // TODO: make_unique
      return std::unique_ptr<http_response>(new cpprest_http_response(response_task.get()));
    }
    catch (const std::exception &e)
    {
      // TODO: handle exceptions
      throw e;
    }
  }

  int cpprest_http_response::last_modified(std::string &last_modified_str) const
  {
    const auto iter = _response.headers().find(U("Last-Modified"));
    if (iter == _response.headers().end())
      return error_code::last_modified_not_found;
    last_modified_str = iter->second;

    const auto last_modified = ::utility::datetime::from_string(last_modified_str);
    if (last_modified.to_interval() == 0)
      return error_code::last_modified_invalid;

    return error_code::success;
  }

  size_t cpprest_http_response::body(char *buffer) const
  {
    // TODO: check buffer not null, or handle exceptions.
    const Concurrency::streams::rawptr_buffer<char> rb(buffer, content_length(), std::ios::out);

    // Write response body into the buffer.
    // TODO: handle exception here?
    return _response.body().read_to_end(rb).get();
  }

} // namespace reinforcement_learning
