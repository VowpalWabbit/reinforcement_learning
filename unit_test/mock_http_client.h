#pragma once
#include "utility/http_client.h"

#include <functional>

using namespace web;
using namespace http;

class mock_http_client : public reinforcement_learning::i_http_client {
public:
  using response_fn = void(const http_request&, http_response&);

  mock_http_client(const std::string& url);

  virtual const std::string& get_url() const override;

  // TODO: Remove the old request method below.
  virtual response_t request(request_t request) override;

  virtual std::unique_ptr<reinforcement_learning::response_base> request(const reinforcement_learning::request_base& request) override;

  void set_responder(http::method, std::function<response_fn> custom_responder);
private:
  static void handle_get(const http_request& message, http_response& resp);
  static void handle_put(const http_request& message, http_response& resp);
  static void handle_post(const http_request& message, http_response& resp);
  static void handle_delete(const http_request& message, http_response& resp);
  static void handle_head(const http_request& message, http_response& resp);

private:
  const std::string _url;
  // TODO: static
  std::map<http::method, std::function<response_fn>> _responders;
};
