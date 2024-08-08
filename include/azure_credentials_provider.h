#pragma once

#ifdef LINK_AZURE_LIBS

#include "api_status.h"
#include "configuration.h"
#include "future_compat.h"

#include "err_constants.h"
#include "future_compat.h"

#include <azure/core/datetime.hpp>
#include <chrono>
#include <mutex>
// These are needed because azure does a bad time conversion
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace reinforcement_learning
{

template<typename T>
class azure_credentials_provider
{
public:
  template<typename... Args>
  azure_credentials_provider(Args&&... args) :
    _creds(std::make_unique<T>(std::forward<Args>(args)...)) {}

  int get_credentials(const std::vector<std::string>& scopes, std::string& token_out,
      std::chrono::system_clock::time_point& expiry_out)
  {
    try
    {
      Azure::Core::Credentials::TokenRequestContext request_context;
      request_context.Scopes = scopes;

      Azure::Core::Context context;
      std::cout << "fetching token for " << scopes[0] << std::endl;

      std::lock_guard<std::mutex> lock(_creds_mtx);
      auto auth = _creds->GetToken(request_context, context);
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

      return 0;
    }
    catch (std::exception& e)
    {
      std::cout << "Error getting auth token: " << e.what();
      return error_code::external_error;
    }
    catch (...)
    {
      std::cout << "Unknown error while getting auth token";
      return error_code::external_error;
    }
    return error_code::success;
  }

private:
  std::unique_ptr<T> _creds;
  mutable std::mutex _creds_mtx;
};

}  // namespace reinforcement_learning

#endif
