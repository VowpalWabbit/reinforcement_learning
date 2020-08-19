#pragma once

#include "rl.net.native.h"
#include "sender.h"
#include "api_status.h"

namespace rl_net_native
{
  using buffer = std::shared_ptr<reinforcement_learning::utility::data_buffer>;
  using error_context = reinforcement_learning::error_callback_fn;
}

extern "C" 
{
  API void ReleaseBufferSharedPointer(const rl_net_native::buffer* buffer);

  API const rl_net_native::buffer* CloneBufferSharedPointer(const rl_net_native::buffer* original);

  API const unsigned char* GetSharedBufferBegin(const rl_net_native::buffer* buffer);
  API const size_t GetSharedBufferLength(const rl_net_native::buffer* buffer); 
}
