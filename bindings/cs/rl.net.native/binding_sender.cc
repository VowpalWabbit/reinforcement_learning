#include "binding_sender.h"
#include <climits>

using namespace reinforcement_learning;

namespace rl_net_native {
  int binding_sender::init(const reinforcement_learning::utility::configuration& config, api_status* status)
  {
    return this->vtable.init(managed_handle, status);
  }

  int binding_sender::v_send(const buffer& data, api_status* status)
  {
    size_t length = data->buffer_filled_size();
    if (length > INT32_MAX)
    {
      RETURN_ERROR_LS(trace_logger, status, background_queue_overflow) << "ISender only supports chunks of up to " << INT32_MAX << " in size.";
    }

    return this->vtable.send(managed_handle, &data, status);
  }

  binding_sender::~binding_sender()
  {
    this->vtable.release(managed_handle);
  }
}
