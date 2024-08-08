#pragma once

#include "rl.net.native.h"
#include "factory_resolver.h"

#include <cstddef>

namespace rl_net_native
{
// Callback function to be called by the C# code to get the OAuth token
// The callback function should return 0 on success and non-zero on failure (see reinforcement_learning::error_code)
// prototype - azure_factory_oauth_callback(const char** scopes, char** token, std::int64_t* expiryUnixTime)
//   scopes - null terminated array of scope strings
//   token - out pointer to a null terminated string allocated on the managed heap
//   expiryUnixTime - pointer to the expiry time of the token in Unix time
typedef int (*azure_factory_oauth_callback_t)(const char**, char**, std::int64_t*);

// Callback function to be called by the C# code to signal the completion of the OAuth token request
// this provides the C# code with the opportunity to free the memory allocated for the token
typedef void (*azure_factory_oauth_callback_complete_t)(char*, int);
}  // namespace rl_net_native

extern "C"
{
  // Register the callback function to be called by the C++ code to get the OAuth token
  // both the callback and completion functions must be registered before any calls to the Azure factories are made
  // typically, this function should be called once during the initialization of the application
  //  callback - the callback function to be called by the C++ code to get the OAuth token
  //  completion - the callback function to be called by the C++ code to signal the completion of the OAuth token request
  API void RegisterDefaultFactoriesCallback(
    rl_net_native::azure_factory_oauth_callback_t callback,
    rl_net_native::azure_factory_oauth_callback_complete_t completion);
}
