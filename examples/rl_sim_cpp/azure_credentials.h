#pragma once

#include "future_compat.h"

#ifdef HAS_STD14
#include <memory>

#include "api_status.h"
#include "configuration.h"

#include <azure/identity/default_azure_credential.hpp>
#include <azure/identity/azure_cli_credential.hpp>

#include <chrono>
#include <string>

class AzureCredentials
{
public:
  AzureCredentials();
  int get_credentials(const std::vector<std::string>& scopes,
      std::string& token_out, std::chrono::system_clock::time_point& expiry_out);
private:
#ifdef HAS_STD14
  Azure::Identity::AzureCliCredentialOptions create_options();

  //Azure::Identity::DefaultAzureCredential _creds;
  Azure::Identity::AzureCliCredential _creds;
  std::string _tenant_id = "<tenant_id>";
#endif
};
#endif