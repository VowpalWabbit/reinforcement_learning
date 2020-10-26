#pragma once

#include <map>

#include "api_status.h"
#include "configuration.h"
#include "data_buffer.h"

namespace reinforcement_learning {
  enum class HttpMethod {
    HEAD,
    GET,
    POST
  };

  // TODO: rename to HttpRequest
  // Generic request class used by all http client library implementations.
  struct request_base {
  public:
    request_base(HttpMethod method) : _method{method} {}

    HttpMethod method() const { return _method; }

    // Adds a header field whose value is read in delay at actual request time.
    void add_header_field(const std::string& name, const char* value_ptr) {
      // TODO: Check duplicates?
      _header_fields[name] = value_ptr;
    }

    // Delayed reading buffer until actual request time.
    void set_body(std::shared_ptr< utility::data_buffer> buffer) {
      _body_buffer = buffer;
    }

  // private:
    std::map<std::string, const char*> _header_fields;
    std::shared_ptr< utility::data_buffer> _body_buffer;
    HttpMethod _method;
  };

  // TODO: rename to HttpResponse
  class response_base {
  public:
    using status_t = short;

    virtual status_t status_code() = 0;
    // Returns empty string if "Last-Modified" is not found in the header.
    virtual std::string last_modified() = 0;
    virtual size_t content_length() = 0;

    // Reads body into buffer. Returns the size.
    // TODO: read twice?
    virtual size_t body(char* buffer) = 0;
  };

  class i_http_client {
  public:
    virtual ~i_http_client() = default;

    // synchronous call, wait until request completed, throwing exceptions in case of failures
    virtual std::unique_ptr<response_base> request(const request_base&) = 0;

    virtual const std::string& get_url() const = 0;
  };

  // Factory for http_client
  int create_http_client(const char* url, const utility::configuration& cfg, i_http_client** client, api_status* status = nullptr);
}
