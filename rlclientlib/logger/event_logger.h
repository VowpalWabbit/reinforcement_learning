#pragma once

#include <stddef.h>

#include "sender.h"
#include "utility/versioned_object_pool.h"

#include "configuration.h"
#include "constants.h"
#include "learning_mode.h"
#include "async_batcher.h"
#include "api_status.h"
#include "error_callback_fn.h"
#include "utility/config_helper.h"
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
    event_logger(i_time_provider* time_provider, i_async_batcher<TEvent>* batcher);

    int init(api_status* status);

  protected:
    int append(TEvent&& item, api_status* status);
    int append(TEvent& item, api_status* status);

  protected:
    bool _initialized = false;
    std::unique_ptr<i_time_provider> _time_provider;

    // Handle batching for the data sent to the eventhub client
    std::unique_ptr<i_async_batcher<TEvent>> _batcher;
  };

  template<typename TEvent>
  event_logger<TEvent>::event_logger(i_time_provider* time_provider, i_async_batcher<TEvent>* batcher) :
      _time_provider(time_provider),
      _batcher(batcher)
  {
  }

  template<typename TEvent>
  int event_logger<TEvent>::init(api_status* status) {
    RETURN_IF_FAIL(_batcher->init(status));
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
    return _batcher->append(item, status);
  }

  template<typename TEvent>
  int event_logger<TEvent>::append(TEvent& item, api_status* status) {
    return append(std::move(item), status);
  }

  class interaction_logger : public event_logger<ranking_event> {
  public:
    interaction_logger(i_time_provider* time_provider, i_async_batcher<ranking_event>* batcher)
      : event_logger(time_provider, batcher)
    {}

    int log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode = ONLINE);
  };

class ccb_logger : public event_logger<decision_ranking_event> {
  public:
    ccb_logger(i_time_provider* time_provider, i_async_batcher<decision_ranking_event>* batcher)
      : event_logger(time_provider, batcher)
    {}

    int log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);
  };

class multi_slot_logger : public event_logger<multi_slot_decision_event> {
  public:
    multi_slot_logger(i_time_provider* time_provider, i_async_batcher<multi_slot_decision_event>* batcher)
      : event_logger(time_provider, batcher)
    {}

    int log_decision(const std::string &event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);
  };

  class observation_logger : public event_logger<outcome_event> {
  public:
    observation_logger(i_time_provider* time_provider, i_async_batcher<outcome_event>* batcher)
      : event_logger(time_provider, batcher)
    {}

    template <typename D>
    int log(const char* event_id, D outcome, api_status* status) {
      const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
      return append(outcome_event::report_outcome(event_id, outcome, now), status);
    }

    int report_action_taken(const char* event_id, api_status* status);
  };

  class generic_event_logger : public event_logger<generic_event> {
  public:
    generic_event_logger(i_time_provider* time_provider, i_async_batcher<generic_event>* batcher)
      : event_logger(time_provider, batcher)
    {}

    int log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, api_status* status);
    int log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, generic_event::action_list_t&& actions, api_status* status);
  };
}}
