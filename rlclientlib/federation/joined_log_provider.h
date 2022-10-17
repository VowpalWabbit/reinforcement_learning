#pragma once

#include "api_status.h"
#include "future_compat.h"
#include "vw/io/io_adapter.h"

#include <cstdint>
#include <string>
#include <vector>

namespace reinforcement_learning
{
struct i_joined_log_batch
{
  virtual ~i_joined_log_batch() = default;

  /// Returns next chunk of batch. chunk_reader will be nullptr when then batch is complete.
  RL_ATTR(nodiscard)
  virtual int next(std::unique_ptr<VW::io::reader>& chunk_reader, api_status* status = nullptr) = 0;
};

/**
 * @brief This interface allows polling access to logged event data.
 */
struct i_joined_log_provider
{
  virtual ~i_joined_log_provider() = default;

  /// Runs the join operation and returns the resulting batch which can be consumed.
  /// The format of the data returned in the batch it implementation dependent.
  RL_ATTR(nodiscard)
  virtual int invoke_join(std::unique_ptr<i_joined_log_batch>& batch, api_status* status = nullptr) = 0;
};
}  // namespace reinforcement_learning
