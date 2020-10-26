#include "cpprest_http_client.h"

#include <chrono>

#include "constants.h"
#include "utility/stl_container_adapter.h"

namespace reinforcement_learning{

const std::map<HttpMethod, web::http::method> http_client::_method_map = {
    {HttpMethod::HEAD, web::http::methods::HEAD},
    {HttpMethod::GET, web::http::methods::GET},
    {HttpMethod::POST, web::http::methods::POST}
};

std::unique_ptr<response_base> http_client::request(const request_base &req)
{
    pplx::task<web::http::http_response> response_task;

    if (req.method() == HttpMethod::POST)
    {
        web::http::http_request request(web::http::methods::POST);
        for (const auto &p : req._header_fields)
            request.headers().add(_XPLATSTR(p.first.c_str()), p.second);

        utility::stl_container_adapter container(req._body_buffer.get());
        const size_t container_size = container.size();
        const auto stream = concurrency::streams::bytestream::open_istream(container);
        request.set_body(stream, container_size);
        response_task = _impl.request(request);
    }
    else
    {
        // HEAD or GET
        response_task = _impl.request(get_cpprest_method(req.method()));
    }
    try
    {
        return std::unique_ptr<cpprest_response>(new cpprest_response(response_task.get()));
    }
    catch (const std::exception &e)
    {
        // TODO: handle exceptions
        throw e;
    }
}

web::http::client::http_client_config get_http_config(const utility::configuration& cfg) {
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

}