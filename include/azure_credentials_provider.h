#pragma once

#ifdef LINK_AZURE_LIBS

#include <chrono>
#include <mutex>
// These are needed because azure does a bad time conversion
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <memory>

#include <azure/core/datetime.hpp>

#ifdef LINK_AZURE_LIBS
#include <azure/core/credentials/credentials.hpp>
#endif

#include "err_constants.h"
#include "trace_logger.h"

namespace reinforcement_learning
{
template<typename T>
class azure_credentials_provider
{
public:
  template<typename... Args>
  azure_credentials_provider(Args&&... args) :
    _creds(std::make_unique<T>(std::forward<Args>(args)...))
  {
  }

  int get_credentials(const std::vector<std::string>& scopes, std::string& token_out,
    std::chrono::system_clock::time_point& expiry_out, i_trace* trace)
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
        TRACE_DEBUG(trace, "azure_credentials_provider: successfully retrieved token");
      }
      token_out = auth.Token;

      // Casting from an azure DateTime object to a time_point does the calculation
      // incorrectly. The expiration is returned as a local time, but the library
      // assumes that it is GMT, and converts the value incorrectly.
      // See: https://github.com/Azure/azure-sdk-for-cpp/issues/5075
      // expiry_out = static_cast<std::chrono::system_clock::time_point>(auth.ExpiresOn);
      std::string dt_string = auth.ExpiresOn.ToString();
      std::tm tm = {};
      std::istringstream ss(dt_string);
      ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
      expiry_out = std::chrono::system_clock::from_time_t(std::mktime(&tm));
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
  std::unique_ptr<T> _creds;
  mutable std::mutex _creds_mtx;
};
}  // namespace reinforcement_learning

#endif
