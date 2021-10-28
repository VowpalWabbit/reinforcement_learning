#pragma once
#include "configuration.h"

namespace reinforcement_learning {
  //this enum sets the behavior of the queue managed by the async_batcher
  enum class queue_mode_enum {
    DROP,//queue drops events if it is full (default)
    BLOCK//queue block if it is full
  };

  // Section constants to be used with get_batcher_config
  const char *const OBSERVATION_SECTION = "observation";
  const char *const INTERACTION_SECTION = "interaction";

namespace utility {
  struct async_batcher_config {
    async_batcher_config();
    int send_high_water_mark;
    int send_batch_interval_ms;
    int send_queue_max_capacity;
    queue_mode_enum queue_mode;
    // bool use_compression;
    // bool use_dedup;
    const char *batch_content_encoding;
    float subsample_rate = 1.f;   // percentage of kept events. 0 = drop all events, 1 = keep all events
  };

  async_batcher_config get_batcher_config(const configuration& config, const char* section);
}}
