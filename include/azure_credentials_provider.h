#pragma once

#ifdef LINK_AZURE_LIBS
#  include "err_constants.h"
#  include "oauth_callback_fn.h"

#  include <azure/core/credentials/credentials.hpp>
#  include <exception>
#  include <memory>
#  include <mutex>

// These are needed because azure does a bad time conversion
#  include <time.h>

#  include <azure/core/datetime.hpp>

namespace reinforcement_learning
{
namespace
{
/**
 * @brief Get the GMT offset from the local time.
 */
inline std::chrono::system_clock::duration get_gmt_offset()
{
  auto get_time_point = [](std::tm& tm)
  {
    // set the tm_isdst field to -1 to let mktime determine if DST is in effect
    tm.tm_isdst = -1;
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
  };
  std::time_t now = std::time(nullptr);
  std::tm local_tm{};
  std::tm gmt_tm{};
#  ifdef _WIN32
  localtime_s(&local_tm, &now);
  gmtime_s(&gmt_tm, &now);
#  else
  localtime_r(&now, &local_tm);
  gmtime_r(&now, &gmt_tm);
#  endif
  return get_time_point(local_tm) - get_time_point(gmt_tm);
}
}  // namespace

/**
 * @brief A template class that provides Azure OAuth credentials.
 *
 * This class is a template that requires a type T which must be a subclass of
 * Azure::Core::Credentials::TokenCredential. It implements the i_oauth_credentials_provider
 * interface to provide OAuth tokens for Azure services.
 *
 * @tparam T The type of the Azure credential, must be a subclass of Azure::Core::Credentials::TokenCredential.
 */
template <typename T>
class azure_credentials_provider : public i_oauth_credentials_provider
{
  static_assert(std::is_base_of<Azure::Core::Credentials::TokenCredential, T>::value,
      "T must be a subclass of Azure::Core::Credentials::TokenCredential");

public:
  /**
   * @brief Constructs an azure_credentials_provider with the given arguments.
   *
   * @tparam Args The types of the arguments to forward to the constructor of T.
   * @param args The arguments to forward to the constructor of T.
   */
  template <typename... Args>
  azure_credentials_provider(Args&&... args) : _creds(std::make_unique<T>(std::forward<Args>(args)...))
  {
    _gmt_offset = get_gmt_offset();
  }

  /**
   * @brief Default destructor.
   */
  ~azure_credentials_provider() override = default;

  /**
   * @brief Retrieves an OAuth token for the given scopes.
   *
   * @param scopes The scopes for which the token is requested.
   * @param token_out The output parameter where the token will be stored.
   * @param expiry_out The output parameter where the token expiry time will be stored.
   * @param trace The trace object for logging.
   * @return int The error code indicating success or failure.
   */
  int get_token(const std::vector<std::string>& scopes, std::string& token_out,
      std::chrono::system_clock::time_point& expiry_out, i_trace* trace) override
  {
    using namespace Azure::Core;
    using namespace Azure::Core::Credentials;
    try
    {
      TokenRequestContext request_context;
      Context context;

      request_context.Scopes = scopes;
      AccessToken auth;
      {
        std::lock_guard<std::mutex> lock(_creds_mtx);
        auth = _creds->GetToken(request_context, context);
      }
      TRACE_DEBUG(trace, "azure_credentials_provider: successfully retrieved token");
      token_out = auth.Token;
      expiry_out = get_expiry_time(auth);
    }
    catch (AuthenticationException& e)
    {
      TRACE_ERROR(trace, e.what());
      return error_code::http_oauth_authentication_error;
    }
    catch (std::exception& e)
    {
      TRACE_ERROR(trace, e.what());
      return error_code::http_oauth_unexpected_error;
    }
    catch (...)
    {
      TRACE_ERROR(trace, "azure_credentials_provider: an unexpected unknown error occurred");
      return error_code::http_oauth_unexpected_error;
    }
    return error_code::success;
  }

private:
  /**
   * @brief Gets the adjusted expiry time of the given access token.
   *
   * @param access_token The access token whose expiry time is to be calculated.
   * @return std::chrono::system_clock::time_point The calculated expiry time.
   * @remarks This function is needed because Azure library returns local time
   *          instead of GMT time.
   */
  std::chrono::system_clock::time_point get_expiry_time(const Azure::Core::Credentials::AccessToken& access_token)
  {
    // Casting from an azure DateTime object to a time_point does the calculation
    // incorrectly. The expiration is returned as a local time, but the library
    // assumes that it is GMT, and converts the value incorrectly.
    // See: https://github.com/Azure/azure-sdk-for-cpp/issues/5075
    return static_cast<std::chrono::system_clock::time_point>(access_token.ExpiresOn) - _gmt_offset;
  }

private:
  std::unique_ptr<T> _creds;
  mutable std::mutex _creds_mtx;
  std::chrono::system_clock::duration _gmt_offset;
};
}  // namespace reinforcement_learning
#endif  // LINK_AZURE_LIBS
