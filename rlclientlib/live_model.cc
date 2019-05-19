#include "action_flags.h"
#include "live_model.h"
#include "live_model_impl.h"
#include "err_constants.h"

#define INIT_CHECK() do {                                       \
  if(!_initialized) {                                           \
    api_status::try_update(status, error_code::not_initialized, \
                "Library not initialized. Call init() first."); \
    return error_code::not_initialized;                         \
  }                                                             \
} while(0);                                                     \

namespace reinforcement_learning
{
	live_model::live_model(
		const utility::configuration& config,
		error_fn fn,
		void* err_context,
		trace_logger_factory_t* trace_factory,
		data_transport_factory_t* t_factory,
		model_factory_t* m_factory,
		sender_factory_t* s_factory,
    time_provider_factory_t* time_prov_factory)
	{
    _pimpl = std::unique_ptr<live_model_impl>(
      new live_model_impl(config, fn, err_context, trace_factory, t_factory, m_factory, s_factory, time_prov_factory));
	}

	live_model::live_model(live_model&& other) {
		std::swap(_pimpl, other._pimpl);
		_initialized = other._initialized;
	}

  live_model::~live_model() = default;

  live_model& live_model::operator=(live_model&& other) {
    std::swap(_pimpl, other._pimpl);
    _initialized = other._initialized;
    return *this;
  }
  int live_model::init(api_status* status) {
    if (_initialized)
      return error_code::success;

    const auto err_code = _pimpl->init(status);
    if (err_code == error_code::success) {
      _initialized = true;
    }

    return err_code;
  }

  int live_model::choose_rank(const char* event_id, const char* context_json, ranking_response& response,
                              api_status* status)
  {
    INIT_CHECK();
    return choose_rank(event_id, context_json, action_flags::DEFAULT, response, status);
  }

  int live_model::choose_rank(const char* context_json, ranking_response& response, api_status* status)
  {
    INIT_CHECK();
    return choose_rank(context_json, action_flags::DEFAULT, response, status);
  }

  //not implemented yet
  int live_model::choose_rank(const char* event_id, const char* context_json, unsigned int flags, ranking_response& response,
    api_status* status)
  {
    INIT_CHECK();
    return _pimpl->choose_rank(event_id, context_json, flags, response, status);
  }

  //not implemented yet
  int live_model::choose_rank(const char* context_json, unsigned int flags, ranking_response& response, api_status* status)
  {
    INIT_CHECK();
    return _pimpl->choose_rank(context_json, flags, response, status);
  }

  //not implemented yet
  int live_model::report_action_taken(const char* event_id, api_status* status) {
    INIT_CHECK();
    return _pimpl->report_action_taken(event_id, status);
  }

  int live_model::report_outcome(const char* event_id, const char* outcome, api_status* status)
  {
    INIT_CHECK();
    return _pimpl->report_outcome(event_id, outcome, status);
  }

  int live_model::report_outcome(const char* event_id, float outcome, api_status* status)
  {
    INIT_CHECK();
    return _pimpl->report_outcome(event_id, outcome, status);
  }

  int live_model::refresh_model(api_status* status)
  {
    INIT_CHECK();
    return _pimpl->refresh_model(status);
  }
}
