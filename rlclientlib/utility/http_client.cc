#include "http_client.h"

#include "cpprest_http_client.h"

namespace reinforcement_learning {

int create_http_client(const char *url, const utility::configuration &cfg,
                       std::unique_ptr<i_http_client> &client,
                       api_status *status) {
  int result = error_code::success;
  try {
    // TODO: Create i_http_client based on cfg options.
    client = std::unique_ptr<i_http_client>(new cpprest_http_client(url, cfg));
  } catch (const std::exception &e) {
    result = error_code::http_client_init_error;
    api_status::try_update(status, result, e.what());
  } catch (...) {
    result = error_code::http_client_init_error;
    api_status::try_update(status, result,
                           error_code::http_client_init_error_s);
  }
  return result;
}

}  // namespace reinforcement_learning
