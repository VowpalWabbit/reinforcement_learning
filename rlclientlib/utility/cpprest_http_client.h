#pragma once

#include <cpprest/http_client.h>

#include "http_client.h"

namespace reinforcement_learning
{

  // TODO: Rename to cpprest_http_response.
  class cpprest_http_response : public http_response
  {
  public:
    // TODO: other ctors.
    cpprest_http_response(web::http::http_response &&response) : _response{std::move(response)} {}

    virtual status_t status_code() const override
    {
      return _response.status_code();
    }

    virtual int last_modified(std::string &last_modified_str) const override;

    virtual size_t content_length() const override
    {
      return _response.headers().content_length();
    }

    virtual size_t body(char *buffer) const override;

  private:
    web::http::http_response _response;
  };

  class cpprest_http_client : public i_http_client
  {
  public:
    cpprest_http_client(const char *url, const utility::configuration &cfg)
        : _url(url),
          _impl(::utility::conversions::to_string_t(url), get_http_config(cfg))
    {
    }

    cpprest_http_client(cpprest_http_client &&other) = delete;
    cpprest_http_client &operator=(cpprest_http_client &&other) = delete;
    cpprest_http_client(const cpprest_http_client &) = delete;
    cpprest_http_client &operator=(const cpprest_http_client &) = delete;

    virtual std::unique_ptr<http_response> request(const http_request &req) override;

    virtual const std::string &get_url() const override
    {
      // TODO: avoid saving url?
      // return _impl.base_uri().to_string();
      return _url;
    }

  private:
    web::http::method get_cpprest_method(http_method method)
    {
      return _method_map.at(method);
    }

    web::http::client::http_client_config get_http_config(const utility::configuration &cfg);

    static const std::map<http_method, web::http::method> _method_map;

    const std::string _url;
    web::http::client::http_client _impl;
  };

} // namespace reinforcement_learning
