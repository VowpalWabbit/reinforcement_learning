#pragma once

#include "api_status.h"
#include "data_buffer.h"
#include "future_compat.h"
#include "vw/io/io_adapter.h"

#include <cstdint>
#include <string>
#include <vector>

namespace reinforcement_learning
{
/**
 * @brief This interface allows polling access to logged event data.
 */
struct i_joined_log_provider
{
  virtual ~i_joined_log_provider() = default;

  // Runs the join operation and returns the resulting batch which can be consumed.
  // The format of the data returned in the batch it implementation dependent.
  RL_ATTR(nodiscard)
  virtual int invoke_join(std::unique_ptr<VW::io::reader>& output, api_status* status = nullptr) = 0;
};
}  // namespace reinforcement_learning
