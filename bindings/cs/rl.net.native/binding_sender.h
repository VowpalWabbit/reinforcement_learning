#pragma once

#include "api_status.h"
#include "configuration.h"
#include "err_constants.h"
#include "error_callback_fn.h"
#include "sender.h"

namespace rl_net_native
{
namespace constants
{
const char* const BINDING_SENDER = "BINDING_SENDER";
}

using buffer = std::shared_ptr<reinforcement_learning::utility::data_buffer>;
using error_context = reinforcement_learning::error_callback_fn;

using error_fn = void (*)(error_context* error_context, reinforcement_learning::api_status* api_status);
using sender_create_fn = void* (*)(const reinforcement_learning::utility::configuration* configuration,
    error_fn error_callback, reinforcement_learning::error_callback_fn* error_ctx);
using sender_init_fn = int (*)(void* managed_handle, reinforcement_learning::api_status* status);
using sender_send_fn = int (*)(void* managed_handle, const buffer* buffer, reinforcement_learning::api_status* status);
using sender_release_fn = void (*)(void* managed_handle);

typedef struct sender_vtable
{
  sender_init_fn init;
  sender_send_fn send;
  sender_release_fn release;
} sender_vtable_t;

class binding_sender : public reinforcement_learning::i_sender
{
public:
  binding_sender(void* managed_handle, sender_vtable_t vtable, reinforcement_learning::i_trace* trace_logger)
      : managed_handle(managed_handle), vtable(vtable), trace_logger(trace_logger){};

  virtual int init(
      const reinforcement_learning::utility::configuration& config, reinforcement_learning::api_status* status);
  virtual ~binding_sender();

protected:
  virtual int v_send(const buffer& data, reinforcement_learning::api_status* status = nullptr);

private:
  void* managed_handle;
  sender_vtable vtable;

  reinforcement_learning::i_trace* trace_logger;
};

}  // namespace rl_net_native
