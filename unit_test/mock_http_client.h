#pragma once

#include <functional>

#include "utility/http_client.h"

class mock_http_response : public reinforcement_learning::http_response
{
public:
  mock_http_response(status_t status_code) : _status_code{status_code} {}

  virtual status_t status_code() const override { return _status_code; }

  virtual int last_modified(std::string &last_modified_str) const override
  {
    // TODO: error_code::success
    return 0;
  }

  virtual size_t content_length() const override { return 0; }

  virtual size_t body(char *buffer) const override { return 0; }

private:
  status_t _status_code;
};

class mock_http_client : public reinforcement_learning::i_http_client
{
public:
  using http_method = reinforcement_learning::http_method;
  using http_request = reinforcement_learning::http_request;
  using http_response = reinforcement_learning::http_response;
  using response_fn = std::unique_ptr<http_response>(const http_request &);

  mock_http_client(const std::string &url) : _url{url} {}

  virtual std::unique_ptr<http_response> request(const http_request &request) override
  {
    auto responder = _responders[request.method()];
    return responder(request);
  }

  virtual const std::string &get_url() const override { return _url; }

  void set_responder(http_method method, std::function<response_fn> custom_responder)
  {
    _responders[method] = custom_responder;
  }

private:
  const std::string _url;
  std::map<http_method, std::function<response_fn>> _responders;
};
