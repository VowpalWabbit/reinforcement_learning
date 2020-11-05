#pragma once

#include <cpprest/http_client.h>

#include "api_status.h"
#include "http_client.h"

namespace reinforcement_learning {

class cpprest_http_response : public http_response {
 public:
  cpprest_http_response(web::http::http_response &&response)
      : _response{std::move(response)} {}

  // Not copyable or movable
  cpprest_http_response(cpprest_http_response &&) = delete;
  cpprest_http_response &operator=(cpprest_http_response &&) = delete;
  cpprest_http_response(const cpprest_http_response &) = delete;
  cpprest_http_response &operator=(const cpprest_http_response &) = delete;

  virtual status_t status_code() const override {
    return _response.status_code();
  }

  virtual int last_modified(std::string &last_modified_str) const override;

  virtual size_t content_length() const override {
    return _response.headers().content_length();
  }

  virtual int body(size_t &size, char *buffer) const override;

 private:
  web::http::http_response _response;
};

class cpprest_http_client : public i_http_client {
 public:
  cpprest_http_client(const char *url, const utility::configuration &cfg)
      : _url(url),
        _impl(::utility::conversions::to_string_t(url), get_http_config(cfg)) {}

  // Not copyable or movable
  cpprest_http_client(cpprest_http_client &&) = delete;
  cpprest_http_client &operator=(cpprest_http_client &&) = delete;
  cpprest_http_client(const cpprest_http_client &) = delete;
  cpprest_http_client &operator=(const cpprest_http_client &) = delete;

  virtual int request(const http_request &request,
                      std::unique_ptr<http_response> &response,
                      api_status *status, i_trace *trace) override;

  virtual const std::string &get_url() const override { return _url; }

  virtual std::string encode(const std::string &uri) const override {
    return ::utility::conversions::to_utf8string(
        web::uri::encode_data_string(::utility::conversions::to_string_t(uri)));
  }

  virtual std::string encode(
      const std::vector<unsigned char> &uri) const override {
    return ::utility::conversions::to_utf8string(
        web::uri::encode_data_string(::utility::conversions::to_base64(uri)));
  }

 private:
  web::http::method get_cpprest_method(http_method method) {
    return _method_map.at(method);
  }

  web::http::client::http_client_config get_http_config(
      const utility::configuration &cfg);

  static const std::map<http_method, web::http::method> _method_map;

  const std::string _url;
  web::http::client::http_client _impl;
};

}  // namespace reinforcement_learning
