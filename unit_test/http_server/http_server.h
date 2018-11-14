#pragma once
#include "utility/http_helper.h"

#include <functional>

using namespace web;
using namespace http;

class mock_http_client : public reinforcement_learning::utility::i_http_client {
public:
  using response_fn = void(const http_request&, http_response&);

  mock_http_client(const std::string& url);

  virtual const std::string& get_url() const override;

  virtual task_t request(method_t method) override;
  virtual task_t request(request_t request) override;

  void set_responder(http::method, std::function<response_fn> custom_responder);
private:
  static void handle_get(const http_request& message, http_response& resp);
  static void handle_put(const http_request& message, http_response& resp);
  static void handle_post(const http_request& message, http_response& resp);
  static void handle_delete(const http_request& message, http_response& resp);
  static void handle_head(const http_request& message, http_response& resp);

private:
  const std::string _url;
  std::map<http::method, std::function<response_fn>> _responders;
};
