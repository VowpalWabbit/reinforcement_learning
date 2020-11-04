#include "restapi_data_transport.h"

#include "api_status.h"
#include "trace_logger.h"

namespace u = reinforcement_learning::utility;

namespace reinforcement_learning {
namespace model_management {

restapi_data_transport::restapi_data_transport(i_http_client *httpcli,
                                               i_trace *trace)
    : _httpcli(httpcli), _datasz{0}, _trace{trace} {}

/*
 * Example successful response
 *
 * Received response status code:200
 * Accept-Ranges = bytes
 * Content-Length = 7666
 * Content-MD5 = VuJg8VgcBQwevGhJR2Yehw==
 * Content-Type = application/octet-stream
 * Date = Mon, 28 May 2018 14:41:02 GMT
 * ETag = "0x8D5C03A2AEC2189"
 * Last-Modified = Tue, 22 May 2018 23:17:20 GMT
 * Server = Windows-Azure-Blob/1.0 Microsoft-HTTPAPI/2.0
 * x-ms-blob-type = BlockBlob
 * x-ms-lease-state = available
 * x-ms-lease-status = unlocked
 * x-ms-request-id = 241f3513-801e-0041-0991-f6893e000000
 * x-ms-server-encrypted = true
 * x-ms-version = 2017-04-17
 */

int restapi_data_transport::get_data_info(std::string &last_modified,
                                          uint64_t &sz, api_status *status) {
  try {
    const auto response = _httpcli->request(http_request(http_method::HEAD));

    if (response->status_code() != http_response::status::OK)
      RETURN_ERROR_ARG(_trace, status, http_bad_status_code,
                       _httpcli->get_url());

    RETURN_IF_FAIL(response->last_modified(last_modified));

    sz = response->content_length();

    return error_code::success;
  } catch (const std::exception &e) {
    RETURN_ERROR_LS(_trace, status, exception_during_http_req)
        << e.what() << "\n URL: " << _httpcli->get_url();
  }
}

int restapi_data_transport::get_data(model_data &ret, api_status *status) {
  std::string curr_last_modified;
  uint64_t curr_datasz;
  RETURN_IF_FAIL(get_data_info(curr_last_modified, curr_datasz, status));

  if (curr_last_modified == _last_modified && curr_datasz == _datasz)
    return error_code::success;

  try {
    const auto response = _httpcli->request(http_request(http_method::GET));

    if (response->status_code() != http_response::status::OK)
      RETURN_ERROR_ARG(_trace, status, http_bad_status_code,
                       "Found: ", response->status_code(), _httpcli->get_url());

    RETURN_IF_FAIL(response->last_modified(curr_last_modified));

    curr_datasz = response->content_length();
    if (curr_datasz > 0) {
      const auto buffer = ret.alloc(curr_datasz);
      const auto readval = response->body(buffer);
      ret.data_sz(readval);
      ret.increment_refresh_count();
      _datasz = readval;
    } else {
      ret.data_sz(0);
    }

    _last_modified = curr_last_modified;
    return error_code::success;
  } catch (const std::exception &e) {
    ret.free();
    RETURN_ERROR_LS(_trace, status, exception_during_http_req) << e.what();
  } catch (...) {
    ret.free();
    RETURN_ERROR_LS(_trace, status, exception_during_http_req)
        << error_code::unknown_s;
  }
}

}  // namespace model_management
}  // namespace reinforcement_learning
