#ifdef LINK_AZURE_LIBS
#include "azure_credentials.h"
#include "err_constants.h"
#include "future_compat.h"

#include <azure/core/datetime.hpp>
#include <chrono>
// These are needed because azure does a bad time conversion
#include <iomanip>
#include <sstream>

#include <exception>
#include <iostream>

using namespace reinforcement_learning;

Azure::Identity::AzureCliCredentialOptions AzureCredentials::create_options()
{
  Azure::Identity::AzureCliCredentialOptions options;
  options.TenantId = _tenant_id;
  options.AdditionallyAllowedTenants.push_back("*");
  return options;
}

AzureCredentials::AzureCredentials()
: _creds(create_options())
{}

int AzureCredentials::get_credentials(const std::vector<std::string>& scopes,
    std::string& token_out, std::chrono::system_clock::time_point& expiry_out)
{
#ifdef HAS_STD14
  Azure::Core::Credentials::TokenRequestContext request_context;
  request_context.Scopes = scopes;
  // TODO: needed?
  request_context.TenantId = _tenant_id;
  Azure::Core::Context context;
  try {
    auto auth = _creds.GetToken(request_context, context);
    token_out = auth.Token;

    // Casting from an azure DateTime object to a time_point does the calculation
    // incorrectly. The expiration is returned as a local time, but the library
    // assumes that it is GMT, and converts the value incorrectly.
    // See: https://github.com/Azure/azure-sdk-for-cpp/issues/5075
    //expiry_out = static_cast<std::chrono::system_clock::time_point>(auth.ExpiresOn);
    std::string dt_string = auth.ExpiresOn.ToString();
    std::tm tm = {};
    std::istringstream ss(dt_string);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    expiry_out = std::chrono::system_clock::from_time_t(std::mktime(&tm));

  }
  catch(std::exception& e){
    std::cout << "Error getting auth token: " << e.what();
    return error_code::external_error;
  }
  catch(...){
    std::cout << "Unknown error while getting auth token";
    return error_code::external_error;
  }
#endif
  return error_code::success;
}
#endif