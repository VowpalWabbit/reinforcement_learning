#pragma once

#include <stddef.h>
#include <functional>
#include <memory>

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
  class i_logger_extensions;

  // This class wraps logging event to event_hub in a generic way that live_model can consume.
  template<typename TFunc>
  class event_logger {
  public:
    event_logger(i_time_provider* time_provider, i_async_batcher<TFunc>* batcher);

    event_logger(i_time_provider* time_provider, i_async_batcher<TFunc>* batcher, const char* app_id);

    int init(api_status* status);

  protected:
    int append(TFunc&& func, const char* evt_id, size_t size_estimate, api_status* status);
    int append(TFunc& func, const char* evt_id, size_t size_estimate, api_status* status);

  protected:
    bool _initialized = false;
    std::unique_ptr<i_time_provider> _time_provider;

    // Handle batching for the data sent to the eventhub client
    std::unique_ptr<i_async_batcher<TFunc>> _batcher;

    const char* _app_id;
  };  

  template<typename TFunc>
  event_logger<TFunc>::event_logger(i_time_provider* time_provider, i_async_batcher<TFunc>* batcher) : event_logger(time_provider, batcher, "") {}

  template<typename TFunc>
  event_logger<TFunc>::event_logger(i_time_provider* time_provider, i_async_batcher<TFunc>* batcher, const char* app_id):
    _time_provider(time_provider),
    _batcher(batcher),
    _app_id(app_id)
  {
  }

  template<typename TFunc>
  int event_logger<TFunc>::init(api_status* status) {
    RETURN_IF_FAIL(_batcher->init(status));
    _initialized = true;
    return error_code::success;
  }

  template<typename TFunc>
  int event_logger<TFunc>::append(TFunc&& func, const char* evt_id, size_t size_estimate, api_status* status) {
    if (!_initialized) {
      api_status::try_update(status, error_code::not_initialized,
        "Logger not initialized. Call init() first.");
      return error_code::not_initialized;
    }

    // Add item to the batch (will be sent later)
    return _batcher->append(item, evt_id, size_estimate, status);
  }

  template<typename TFunc>
  int event_logger<TFunc>::append(TFunc& func, const char* evt_id, size_t size_estimate, api_status* status) {
    return append(std::move(item), evt_id, size_estimate, status);
  }

  class interaction_logger : public event_logger<std::function<int(ranking_event&, api_status*)>> {
  public:
    interaction_logger(i_time_provider* time_provider, i_async_batcher<std::function<int(ranking_event&, api_status*)>>* batcher)
      : event_logger(time_provider, batcher)
    {}

    int log(const char* event_id, const char* context, unsigned int flags, const ranking_response& response, api_status* status, learning_mode learning_mode = ONLINE);
  };

class ccb_logger : public event_logger<std::function<int(decision_ranking_event&, api_status*)>> {
  public:
    ccb_logger(i_time_provider* time_provider, i_async_batcher<std::function<int(decision_ranking_event&, api_status*)>>* batcher)
      : event_logger(time_provider, batcher)
    {}

    int log_decisions(std::vector<const char*>& event_ids, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);
  };

class multi_slot_logger : public event_logger<std::function<int(multi_slot_decision_event&, api_status*)>> {
  public:
    multi_slot_logger(i_time_provider* time_provider, i_async_batcher<std::function<int(multi_slot_decision_event&, api_status*)>>* batcher)
      : event_logger(time_provider, batcher)
    {}

    int log_decision(const std::string &event_id, const char* context, unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, api_status* status);
  };

  class observation_logger : public event_logger<std::function<int(outcome_event&, api_status*)>> {
  public:
    observation_logger(i_time_provider* time_provider, i_async_batcher<std::function<int(outcome_event&, api_status*)>>* batcher)
      : event_logger(time_provider, batcher)
    {}

    template <typename D>
    int log(const char* event_id, D outcome, api_status* status) {
      const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
      return append(outcome_event::report_outcome(event_id, outcome, now), status);
    }

    int report_action_taken(const char* event_id, api_status* status);
  };

  class generic_event_logger : public event_logger<std::function<int(generic_event&, api_status*)>> {
  public:
    generic_event_logger(i_time_provider* time_provider, i_async_batcher<std::function<int(generic_event&, api_status*)>>* batcher, const char* app_id)
      : event_logger(time_provider, batcher, app_id)
    {}

    template<typename TSerializer, typename... Args>
    int log(const char* event_id, const char* context, generic_event::payload_type_t type, i_logger_extensions* ext, TSerializer& serializer, api_status* status, Args... args) {
      const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
      // using std::bind for the in_evt to get in-place construction.
      // We can replace this with a pure lambda in C++14
      using namespace std::placeholders;
      auto evt_fn =
        std::bind(
          [type, ext, serializer, args...](generic_event& out_evt, api_status* status, generic_event in_evt)->int
          {
            RETURN_IF_FAIL(in_evt.transform(ext, serializer, status, args...));
            out_evt = std::move(in_evt);
            return error_code::success;
          },
          _1,
          _2,
          generic_event(event_id, now, type, context, _app_id)
        );
      return append(std::move(evt_fn), event_id, 1, status);
    }

    // TODO: used for observations for now.. may want to change that later
    // These functions will take in fully transformed generic_event objects, and should only be used
    // when the creation of those types are very cheap
    int log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, event_content_type content_type, api_status* status);
    int log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type, event_content_type content_type, generic_event::object_list_t&& objects, api_status* status);
  };
}}
