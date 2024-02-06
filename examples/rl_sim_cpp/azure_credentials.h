#pragma once

#ifdef LINK_AZURE_LIBS
#include <memory>

#include "api_status.h"
#include "configuration.h"
#include "future_compat.h"

#include <azure/identity/default_azure_credential.hpp>
#include <azure/identity/azure_cli_credential.hpp>

#include <chrono>
#include <string>

class AzureCredentials
{
public:
  AzureCredentials(const std::string& tenant_id);
  int get_credentials(const std::vector<std::string>& scopes,
      std::string& token_out, std::chrono::system_clock::time_point& expiry_out);
private:
  std::string _tenant_id;
#ifdef HAS_STD14
  Azure::Identity::AzureCliCredentialOptions create_options();

  //Azure::Identity::DefaultAzureCredential _creds;
  Azure::Identity::AzureCliCredential _creds;
#endif
};
#endif