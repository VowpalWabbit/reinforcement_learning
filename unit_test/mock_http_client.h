#pragma once

#include <functional>
#include <string>

#include "api_status.h"
#include "utility/http_client.h"

class mock_http_response : public reinforcement_learning::http_response {
 public:
  mock_http_response(status_t status_code) : _status_code{status_code} {}

  mock_http_response(status_t status_code, const std::string &last_modified,
                     const std::string &body)
      : _status_code{status_code}, _last_modified{last_modified}, _body{body} {}

  virtual status_t status_code() const override { return _status_code; }

  virtual int last_modified(std::string &last_modified_str) const override {
    last_modified_str = _last_modified;
    return reinforcement_learning::error_code::success;
  }

  virtual size_t content_length() const override { return _body.length(); }

  virtual size_t body(char *buffer) const override {
    _body.copy(buffer, _body.length());
    return _body.length();
  }

 private:
  status_t _status_code;
  std::string _last_modified;
  std::string _body;
};

class mock_http_client : public reinforcement_learning::i_http_client {
 public:
  using http_method = reinforcement_learning::http_method;
  using http_request = reinforcement_learning::http_request;
  using http_response = reinforcement_learning::http_response;
  using response_fn = std::unique_ptr<http_response>(const http_request &);

  mock_http_client(const std::string &url) : _url{url} {}

  virtual std::unique_ptr<http_response> request(
      const http_request &request) override {
    auto responder = _responders[request.method()];
    return responder(request);
  }

  virtual const std::string &get_url() const override { return _url; }

  virtual std::string encode(const std::string &uri) const override {
    return uri;
  }

  virtual std::string encode(
      const std::vector<unsigned char> &uri) const override {
    return std::string(uri.begin(), uri.end());
  }

  void set_responder(http_method method,
                     std::function<response_fn> custom_responder) {
    _responders[method] = custom_responder;
  }

 private:
  std::map<http_method, std::function<response_fn>> _responders;
  const std::string _url;
};
