#include "live_model_episodic.h"

#include "action_flags.h"
#include "err_constants.h"
#include "live_model_impl.h"

#define INIT_CHECK()                                                                                     \
  do {                                                                                                   \
    if (!_initialized)                                                                                   \
    {                                                                                                    \
      RETURN_ERROR_ARG(nullptr, status, not_initialized, "Library not initialized. Call init() first."); \
    }                                                                                                    \
  } while (0);

namespace reinforcement_learning
{
live_model_episodic::live_model_episodic(const utility::configuration& config, error_fn fn, void* err_context,
    trace_logger_factory_t* trace_factory, data_transport_factory_t* t_factory, model_factory_t* m_factory,
    sender_factory_t* s_factory, time_provider_factory_t* time_prov_factory)
{
  _pimpl = std::unique_ptr<live_model_impl>(
      new live_model_impl(config, fn, err_context, trace_factory, t_factory, m_factory, s_factory, time_prov_factory));
}

live_model_episodic::live_model_episodic(const utility::configuration& config, std::function<void(const api_status&)> error_cb,
    trace_logger_factory_t* trace_factory, data_transport_factory_t* t_factory, model_factory_t* m_factory,
    sender_factory_t* s_factory, time_provider_factory_t* time_prov_factory)
{
  _pimpl = std::unique_ptr<live_model_impl>(new live_model_impl(
      config, std::move(error_cb), trace_factory, t_factory, m_factory, s_factory, time_prov_factory));
}

live_model_episodic::live_model_episodic(live_model_episodic&& other) noexcept
{
  std::swap(_pimpl, other._pimpl);
  _initialized = other._initialized;
}

live_model_episodic::~live_model_episodic() = default;

live_model_episodic& live_model_episodic::operator=(live_model_episodic&& other) noexcept
{
  std::swap(_pimpl, other._pimpl);
  _initialized = other._initialized;
  return *this;
}

int live_model_episodic::init(api_status* status)
{
  if (_initialized) { return error_code::success; }

  const auto err_code = _pimpl->init(status);
  if (err_code == error_code::success) { _initialized = true; }

  return err_code;
}

std::vector<int> live_model_episodic::c_array_to_vector(const int* c_array, size_t array_size)
{
  if (c_array == nullptr) { return {}; }
  return std::vector<int>(c_array, c_array + array_size);
}

// not implemented yet
int live_model_episodic::report_action_taken(const char* event_id, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_action_taken(event_id, status);
}

int live_model_episodic::report_outcome(const char* primary_id, int secondary_id, const char* outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(primary_id, secondary_id, outcome, status);
}

int live_model_episodic::report_outcome(const char* primary_id, int secondary_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(primary_id, secondary_id, outcome, status);
}

int live_model_episodic::report_outcome(
    const char* primary_id, const char* secondary_id, const char* outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(primary_id, secondary_id, outcome, status);
}

int live_model_episodic::report_outcome(const char* primary_id, const char* secondary_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(primary_id, secondary_id, outcome, status);
}

int live_model_episodic::refresh_model(api_status* status)
{
  INIT_CHECK();
  return _pimpl->refresh_model(status);
}

int live_model_episodic::request_episodic_decision(const char* event_id, const char* previous_id, string_view context_json,
    ranking_response& resp, episode_state& episode, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_episodic_decision(
      event_id, previous_id, context_json, action_flags::DEFAULT, resp, episode, status);
}

int live_model_episodic::request_episodic_decision(const char* event_id, const char* previous_id, string_view context_json,
    unsigned int flags, ranking_response& resp, episode_state& episode, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_episodic_decision(event_id, previous_id, context_json, flags, resp, episode, status);
}

int live_model_episodic::report_action_taken(const char* primary_id, const char* secondary_id, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_action_taken(primary_id, secondary_id, status);
}

}  // namespace reinforcement_learning
