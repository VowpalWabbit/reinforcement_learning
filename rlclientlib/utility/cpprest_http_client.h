#pragma once

#include "http_client.h"

#include <cpprest/http_client.h>
#include <cpprest/asyncrt_utils.h>
#include <cpprest/rawptrstream.h>

#include "configuration.h"

namespace reinforcement_learning
{
    class cpprest_response : public response_base
    {
    public:
        cpprest_response(web::http::http_response &&response) : _response{std::move(response)} {}

        virtual status_t status_code() override
        {
            return _response.status_code();
        }

        virtual std::string last_modified() override
        {
            const auto iter = _response.headers().find(U("Last-Modified"));
            if (iter == _response.headers().end())
                return "";
            return iter->second;
        }

        virtual size_t content_length() override
        {
            return _response.headers().content_length();
        }

        virtual size_t body(char *buffer) override
        {
            // TODO: check buffer not null, or handle exceptions.
            const Concurrency::streams::rawptr_buffer<char> rb(buffer, content_length(), std::ios::out);

            // Write response body into the buffer.
            // TODO: handle exception here?
            return _response.body().read_to_end(rb).get();
        }

    private:
        web::http::http_response _response;
    };

    // TODO: rename to cpprest_http_client
    class http_client : public i_http_client
    {
    public:
        http_client(const char *url, const utility::configuration &cfg)
            : _url(url),
              _impl(::utility::conversions::to_string_t(url), get_http_config(cfg))
        {
        }

        http_client(http_client &&other) = delete;
        http_client &operator=(http_client &&other) = delete;
        http_client(const http_client &) = delete;
        http_client &operator=(const http_client &) = delete;

        virtual std::unique_ptr<response_base> request(const request_base &req) override;

        virtual const std::string &get_url() const override
        {
            // TODO: avoid saving url?
            // return _impl.base_uri().to_string();
            return _url;
        }

    private:
        web::http::method get_cpprest_method(HttpMethod method)
        {
            return _method_map.at(method);
        }

        web::http::client::http_client_config get_http_config(const utility::configuration& cfg);

        static const std::map<HttpMethod, web::http::method> _method_map;

        const std::string _url;
        web::http::client::http_client _impl;
    };

} // namespace reinforcement_learning