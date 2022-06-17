#pragma once

#include "api_status.h"
#include "async_batcher.h"
#include "configuration.h"
#include "constants.h"
#include "error_callback_fn.h"
#include "learning_mode.h"
#include "message_sender.h"
#include "ranking_event.h"
#include "ranking_response.h"
#include "rl_string_view.h"
#include "sender.h"
#include "serialization/fb_serializer.h"
#include "time_helper.h"
#include "utility/config_helper.h"
#include "utility/versioned_object_pool.h"
#include "utility/watchdog.h"

#include <stddef.h>

#include <functional>
#include <memory>

namespace reinforcement_learning
{
namespace logger
{
class i_logger_extensions;

// This class wraps logging event to event_hub in a generic way that live_model can consume.
template <typename TEvent>
class event_logger
{
public:
  using TFunc = std::function<int(TEvent&, api_status*)>;

public:
  event_logger(i_time_provider* time_provider, i_async_batcher<TEvent>* batcher);

  event_logger(i_time_provider* time_provider, i_async_batcher<TEvent>* batcher, const char* app_id);

  int init(api_status* status);

protected:
  int append(TFunc&& func, TEvent* event, api_status* status);
  int append(TFunc& func, TEvent* event, api_status* status);

protected:
  bool _initialized = false;
  std::unique_ptr<i_time_provider> _time_provider;

  // Handle batching for the data sent to the eventhub client
  std::unique_ptr<i_async_batcher<TEvent>> _batcher;

  const char* _app_id;
};

template <typename TEvent>
event_logger<TEvent>::event_logger(i_time_provider* time_provider, i_async_batcher<TEvent>* batcher)
    : event_logger(time_provider, batcher, "")
{
}

template <typename TEvent>
event_logger<TEvent>::event_logger(i_time_provider* time_provider, i_async_batcher<TEvent>* batcher, const char* app_id)
    : _time_provider(time_provider), _batcher(batcher), _app_id(app_id)
{
}

template <typename TEvent>
int event_logger<TEvent>::init(api_status* status)
{
  RETURN_IF_FAIL(_batcher->init(status));
  _initialized = true;
  return error_code::success;
}

template <typename TEvent>
int event_logger<TEvent>::append(TFunc&& func, TEvent* event, api_status* status)
{
  if (!_initialized)
  {
    api_status::try_update(status, error_code::not_initialized, "Logger not initialized. Call init() first.");
    return error_code::not_initialized;
  }

  // Add item to the batch (will be sent later)
  return _batcher->append(func, event, status);
}

template <typename TEvent>
int event_logger<TEvent>::append(TFunc& func, TEvent* event, api_status* status)
{
  return append(std::move(func), status);
}

class interaction_logger : public event_logger<ranking_event>
{
public:
  interaction_logger(i_time_provider* time_provider, i_async_batcher<ranking_event>* batcher)
      : event_logger(time_provider, batcher)
  {
  }

  int log(const char* event_id, string_view context, unsigned int flags, const ranking_response& response,
      api_status* status, learning_mode learning_mode = ONLINE);
};

class ccb_logger : public event_logger<decision_ranking_event>
{
public:
  ccb_logger(i_time_provider* time_provider, i_async_batcher<decision_ranking_event>* batcher)
      : event_logger(time_provider, batcher)
  {
  }

  int log_decisions(std::vector<const char*>& event_ids, string_view context, unsigned int flags,
      const std::vector<std::vector<uint32_t>>& action_ids, const std::vector<std::vector<float>>& pdfs,
      const std::string& model_version, api_status* status);
};

class multi_slot_logger : public event_logger<multi_slot_decision_event>
{
public:
  multi_slot_logger(i_time_provider* time_provider, i_async_batcher<multi_slot_decision_event>* batcher)
      : event_logger(time_provider, batcher)
  {
  }

  int log_decision(const std::string& event_id, string_view context, unsigned int flags,
      const std::vector<std::vector<uint32_t>>& action_ids, const std::vector<std::vector<float>>& pdfs,
      const std::string& model_version, api_status* status);
};

class observation_logger : public event_logger<outcome_event>
{
public:
  observation_logger(i_time_provider* time_provider, i_async_batcher<outcome_event>* batcher)
      : event_logger(time_provider, batcher)
  {
  }

  template <typename D>
  int log(const char* event_id, D outcome, api_status* status)
  {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    // A gross little shuffle because report_outcome returns a copy, but we actually need a pointer
    auto evt_sp = std::make_shared<outcome_event>();
    auto evt_copy = outcome_event::report_outcome(event_id, outcome, now);
    *evt_sp = std::move(evt_copy);
    auto evt_fn = [evt_sp](outcome_event& out_evt, api_status* status) -> int
    {
      out_evt = std::move(*evt_sp);
      return error_code::success;
    };
    return append(std::move(evt_fn), evt_sp.get(), status);
  }

  int report_action_taken(const char* event_id, api_status* status);
};

class generic_event_logger : public event_logger<generic_event>
{
public:
  generic_event_logger(i_time_provider* time_provider, i_async_batcher<generic_event>* batcher, const char* app_id)
      : event_logger(time_provider, batcher, app_id)
  {
  }

  template <typename TSerializer, typename... Args>
  int log(const char* event_id, string_view context, generic_event::payload_type_t type, i_logger_extensions* ext,
      TSerializer& serializer, api_status* status, const Args&... args)
  {
    const auto now = _time_provider != nullptr ? _time_provider->gmt_now() : timestamp();
    // using shared_ptr because we can't move a unique_ptr in C++11
    // We should replace them in C++14
    auto evt_sp = std::make_shared<generic_event>(event_id, now, type, context, _app_id);
    // there's no guarantee that the parameter pack Args will stay in scope, so we need to capture them
    // as a copy the ensure their lifetime.
    // TODO: See if there's a way to do this without the copy, since the pack can contain some pretty
    //       expensive objects
    auto evt_fn = [evt_sp, type, ext, serializer, args...](generic_event& out_evt, api_status* status) -> int
    {
      RETURN_IF_FAIL(evt_sp->transform(ext, serializer, status, args...));
      out_evt = std::move(*evt_sp);
      return error_code::success;
    };
    return append(std::move(evt_fn), evt_sp.get(), status);
  }

  // TODO: used for observations for now.. may want to change that later
  // These functions will take in fully transformed generic_event objects, and should only be used
  // when the creation of those types are very cheap
  int log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type,
      event_content_type content_type, api_status* status);
  int log(const char* event_id, generic_event::payload_buffer_t&& payload, generic_event::payload_type_t type,
      event_content_type content_type, generic_event::object_list_t&& objects, api_status* status);
};
}  // namespace logger
}  // namespace reinforcement_learning
