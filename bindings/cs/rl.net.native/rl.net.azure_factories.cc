#include "rl.net.azure_factories.h"

namespace rl_net_native
{
rl_net_native::azure_factory_oauth_callback_t g_oauth_callback = nullptr;
rl_net_native::azure_factory_oauth_callback_complete_t g_oauth_callback_complete = nullptr;

static int azure_factory_oauth_callback(const std::vector<std::string>& scopes,
    std::string& oauth_token, std::chrono::system_clock::time_point& token_expiry)
{
  if (g_oauth_callback == nullptr) {
    return -1;
  }
  // create a null terminated array of scope string pointers
  // these are pointers are readonly and owned by the caller
  std::vector<const char*> native_scopes;
  native_scopes.reserve(scopes.size() + 1);
  for (int i = 0; i < scopes.size(); ++i)
  {
    native_scopes.push_back(scopes[i].c_str());
  }
  native_scopes.push_back(nullptr);
  // we expect to get a pointer to a null terminated string
  // it's allocated on the managed heap, so we can't free it here
  // instead, we will pass it back to the managed code after we copy it
  char *oauth_token_ptr = nullptr;
  std::int64_t expiryUnixTime = 0;
  auto ret = g_oauth_callback(native_scopes.data(), &oauth_token_ptr, &expiryUnixTime);
  if (ret == reinforcement_learning::error_code::success) {
    oauth_token = oauth_token_ptr;
    token_expiry = std::chrono::system_clock::from_time_t(expiryUnixTime);
  }
  g_oauth_callback_complete(oauth_token_ptr, reinforcement_learning::error_code::success);
  return ret;
}
}

API void RegisterDefaultFactoriesCallback(rl_net_native::azure_factory_oauth_callback_t callback,
  rl_net_native::azure_factory_oauth_callback_complete_t completion)
{
  if (rl_net_native::g_oauth_callback == nullptr) {
    rl_net_native::g_oauth_callback = callback;
    reinforcement_learning::register_default_factories_callback(
        reinforcement_learning::oauth_callback_t{rl_net_native::azure_factory_oauth_callback});
    rl_net_native::g_oauth_callback_complete = completion;
  }
}
