#pragma once

#include <map>
#include <memory>

#include "api_status.h"
#include "configuration.h"
#include "data_buffer.h"

namespace reinforcement_learning {
enum class http_method { HEAD, GET, POST };

// Generic request class used by all http client library implementations.
// Setters are lazy methods to avoid data copying.
struct http_request {
 public:
  http_request(http_method method) : _method{method} {}

  http_method method() const { return _method; }

  // Sets a header field whose value is read in delay at actual request time.
  void set_header_field(const std::string &name, const char *value_ptr) {
    _header_fields[name] = value_ptr;
  }

  // Sets the buffer of body content, delayed reading at actual request time.
  void set_body(std::shared_ptr<utility::data_buffer> buffer) {
    _body_buffer = buffer;
  }

  std::map<std::string, const char *> _header_fields;
  std::shared_ptr<utility::data_buffer> _body_buffer;
  http_method _method;
};

class http_response {
 public:
  virtual ~http_response() = default;

  using status_t = short;
  enum status : status_t { OK = 200, CREATED = 201, INTERNAL_ERROR = 500 };

  virtual status_t status_code() const = 0;

  // Returns error_code if "Last-Modified" is not found or invalid.
  virtual int last_modified(std::string &last_modified_str) const = 0;

  // Returns Content-Length.
  virtual size_t content_length() const = 0;

  // Reads body into buffer. Returns the size.
  virtual size_t body(char *buffer) const = 0;
};

class i_http_client {
 public:
  virtual ~i_http_client() = default;

  // Synchronous call, wait until request completed, throwing exceptions in case
  // of failures
  virtual std::unique_ptr<http_response> request(const http_request &) = 0;

  virtual const std::string &get_url() const = 0;

  virtual std::string encode(const std::string &) const = 0;
  virtual std::string encode(const std::vector<unsigned char> &) const = 0;
};

// Factory for http_client
int create_http_client(const char *url, const utility::configuration &cfg,
                       i_http_client **client, api_status *status = nullptr);

}  // namespace reinforcement_learning
