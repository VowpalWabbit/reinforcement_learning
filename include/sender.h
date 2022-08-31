#pragma once
#include "configuration.h"
#include "data_buffer.h"

#include <memory>
namespace reinforcement_learning
{
class api_status;
class i_sender
{
public:
  using buffer = std::shared_ptr<utility::data_buffer>;
  virtual int init(const utility::configuration& config, api_status* status) = 0;

  // For mocking in unit tests, buffer& data may be initialized with nullptr
  // Disable UBSan here to prevent generating an error
#ifdef RL_USE_UBSAN
  __attribute__((no_sanitize("undefined")))
#endif
  int send(const buffer& data, api_status* status = nullptr)
  {
    return v_send(data, status);
  }

  virtual ~i_sender() = default;

protected:
  virtual int v_send(const buffer& data, api_status* status = nullptr) = 0;
};
}  // namespace reinforcement_learning
