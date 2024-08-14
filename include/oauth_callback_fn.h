#pragma once

#include "trace_logger.h"

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace reinforcement_learning
{
class i_oauth_credentials_provider
{
public:
  virtual int get_token(const std::vector<std::string>& scopes, std::string& token_out,
      std::chrono::system_clock::time_point& expiry_out, i_trace* trace) = 0;
  virtual ~i_oauth_credentials_provider() {}
};

using oauth_callback_t = std::function<int(
    const std::vector<std::string>&, std::string&, std::chrono::system_clock::time_point&, i_trace* trace)>;

/**
 * @brief Wraps a callback function into an i_oauth_credentials_provider interface
 * @remark azure_cred_provider_cb_wrapper was introduced as part of a set of changes
 *         that unify the way we handle Azure credentials by following the same pattern
 *         using the factory resolver. This class allows us to avoid breaking changes
 *         with respect to the existing codebase.
 */
class oauth_cred_provider_cb_wrapper : public i_oauth_credentials_provider
{
public:
  /**
   *  @brief Constructor
   * @param cb The callback function to be wrapped
   * @remark This wrapper keeps a reference to the callback function. Take note of the
   *         lifetime of the callback function to avoid disaster.
   */
  oauth_cred_provider_cb_wrapper(oauth_callback_t& cb) : _cb(cb) {}
  ~oauth_cred_provider_cb_wrapper() override = default;

  int get_token(const std::vector<std::string>& scopes, std::string& token_out,
      std::chrono::system_clock::time_point& expiry_out, i_trace* trace) override
  {
    return _cb(scopes, token_out, expiry_out, trace);
  }

private:
  oauth_callback_t& _cb;
};
}  // namespace reinforcement_learning