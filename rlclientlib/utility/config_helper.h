#pragma once
#include "configuration.h"

namespace reinforcement_learning
{
// this enum sets the behavior of the queue managed by the async_batcher
enum class queue_mode_enum
{
  DROP,  // queue drops events if it is full (default)
  BLOCK  // queue block if it is full
};

// this enum sets the counter for number of events behaviour in aysnc_batcher
enum class events_counter_status
{
  ENABLE,  // counter is enabled only if number of actual events received have to be counted
  DISABLE  // counter by default is disabled
};

#ifdef RL_BUILD_FEDERATION
enum class model_payload_type_enum
{
  FULL,  // full model (default)
  DELTA  // model delta
};

model_payload_type_enum to_model_payload_type_enum(const char* model_payload_type);
#endif

// Section constants to be used with get_batcher_config
const char* const OBSERVATION_SECTION = "observation";
const char* const INTERACTION_SECTION = "interaction";

namespace utility
{
struct async_batcher_config
{
  async_batcher_config();
  int send_high_water_mark;
  int send_batch_interval_ms;
  int send_queue_max_capacity;
  queue_mode_enum queue_mode;
  // bool use_compression;
  // bool use_dedup;
  const char* batch_content_encoding{};
  float subsample_rate = 1.f;  // percentage of kept events. 0 = drop all events, 1 = keep all events
  events_counter_status event_counter_status;
};

async_batcher_config get_batcher_config(const configuration& config, const char* section);
events_counter_status get_counter_status(const configuration& config, const char* section);
}  // namespace utility
}  // namespace reinforcement_learning
