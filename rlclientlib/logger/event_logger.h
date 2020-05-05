#pragma once

#include <stddef.h>

#include "sender.h"
#include "utility/versioned_object_pool.h"

#include "configuration.h"
#include "constants.h"
#include "learning_mode.h"
#include "async_batcher.h"
#include "eventhub_client.h"
#include "api_status.h"
#include "../error_callback_fn.h"
#include "utility/watchdog.h"
#include "ranking_response.h"
#include "ranking_event.h"

#include "serialization/fb_serializer.h"
#include "message_sender.h"
#include "time_helper.h"
namespace reinforcement_learning { namespace logger {
  // This class wraps logging event to event_hub in a generic way that live_model can consume.
  template<typename TEvent>
  class event_logger {
  public:
    event_logger(
      i_message_sender* sender,
      int send_high_watermark,
      int send_batch_interval_ms,
      int send_queue_max_capacity,
      const char* queue_mode,
      utility::watchdog& watchdog,
      i_time_provider* time_provider,
      error_callback_fn* perror_cb = nullptr);

    int init(api_status* status);

  protected:
    int append(TEvent&& item, api_status* status);
    int append(TEvent& item, api_status* status);

  protected:
    bool _initialized = false;
    std::unique_ptr<i_time_provider> _time_provider;

    // Handle batching for the data sent to the eventhub client
    async_batcher<TEvent, fb_collection_serializer> _batcher;
  };

  template<typename TEvent>
  event_logger<TEvent>::event_logger(
    i_message_sender* sender,
    int send_high_watermark,
    int send_batch_interval_ms,
    int send_queue_max_capacity,
    const char* queue_mode,
    utility::watchdog& watchdog,
    i_time_provider* time_provider,
    error_callback_fn* perror_cb
  )
    : _batcher(
      sender,
      watchdog,
      perror_cb,
      send_high_watermark,
      send_batch_interval_ms,
      send_queue_max_capacity,
      to_queue_mode_enum(queue_mode)),
      _time_provider(time_provider)
  {}

  template<typename TEvent>
  int event_logger<TEvent>::init(api_status* status) {
    RETURN_IF_FAIL(_batcher.init(status));
    _initialized = true;
    return error_code::success;
  }

  template<typename TEvent>
  int event_logger<TEvent>::append(TEvent&& item, api_status* status) {
    if (!_initialized) {
      api_status::try_update(status, error_code::not_initialized,
        "Logger not initialized. Call init() first.");
      return error_code::not_initialized;
    }

    // Add item to the batch (will be sent later)
    return _batcher.append(item, status);
  }

  template<typename TEvent>
  int event_logger<TEvent>::append(TEvent& item, api_status* status) {
    return append(std::move(item), status);
  }

  class interaction_logger : public event_logger<ranking_event> {
  public:
    interaction_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider,error_callback_fn* perror_cb = nullptr)
      : event_logger(
        sender,
        c.get_int(name::INTERACTION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::INTERACTION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
        c.get(name::QUEUE_MODE, "DROP"),
        watchdog,
        time_provider,
        perror_cb)
    {}

    int log(const char* context, unsigned int flags, learning_mode learning_mode, const ranking_response& response, api_status* status);
  };

  class ccb_logger : public event_logger<decision_ranking_event> {
  public:
    ccb_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr)
      : event_logger(
        sender,
        c.get_int(name::DECISION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::DECISION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::DECISION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
        c.get(name::QUEUE_MODE, "DROP"),
        watchdog,
        time_provider,
        perror_cb)
    {}

    int log(const char* context, unsigned int flags, const decision_response& response, api_status* status);
  };

  class observation_logger : public event_logger<outcome_event> {
  public:
    observation_logger(const utility::configuration& c, i_message_sender* sender, utility::watchdog& watchdog, i_time_provider* time_provider, error_callback_fn* perror_cb = nullptr)
      : event_logger(
        sender,
        c.get_int(name::OBSERVATION_SEND_HIGH_WATER_MARK, 198 * 1024),
        c.get_int(name::OBSERVATION_SEND_BATCH_INTERVAL_MS, 1000),
        c.get_int(name::OBSERVATION_SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024,
        c.get(name::QUEUE_MODE, "DROP"),
        watchdog,
        time_provider,
        perror_cb)
    {}

    template <typename D>
    int log(const char* event_id, D outcome, api_status* status) {
      const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
      return append(outcome_event::report_outcome(event_id, outcome, now), status);
    }

    int report_action_taken(const char* event_id, api_status* status);
  };
}}
