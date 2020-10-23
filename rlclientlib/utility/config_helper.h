#pragma once
#include "configuration.h"

namespace reinforcement_learning {
  //this enum sets the behavior of the queue managed by the async_batcher
  enum class queue_mode_enum {
    DROP,//queue drops events if it is full (default)
    BLOCK//queue block if it is full
  };

  enum class content_encoding_enum {
    IDENTITY,
    ZSTD_AND_DEDUP
  };

  // Section constants to be used with get_batcher_config
  const char *const OBSERVATION_SECTION = "observation";
  const char *const INTERACTION_SECTION = "interaction";

  queue_mode_enum to_queue_mode_enum(const char* queue_mode);
  const char* to_queue_mode_string(queue_mode_enum queue_mode);

  content_encoding_enum to_content_encoding_enum(const char *content_encoding);
  const char* to_content_encoding_string(content_encoding_enum content_encoding);

namespace utility {
  struct async_batcher_config {
    async_batcher_config();
    int send_high_water_mark;
    int send_batch_interval_ms;
    int send_queue_max_capacity;
    queue_mode_enum queue_mode;
    content_encoding_enum content_encoding;
  };

  async_batcher_config get_batcher_config(const configuration& config, const char* section);
}}
