#include "live_model_base.h"

#include "action_flags.h"
#include "err_constants.h"
#include "live_model_impl.h"

namespace reinforcement_learning
{
live_model_base::live_model_base(const utility::configuration& config, error_fn fn, void* err_context,
    trace_logger_factory_t* trace_factory, data_transport_factory_t* t_factory, model_factory_t* m_factory,
    sender_factory_t* s_factory, time_provider_factory_t* time_prov_factory)
{
  _pimpl = std::unique_ptr<live_model_impl>(
      new live_model_impl(config, fn, err_context, trace_factory, t_factory, m_factory, s_factory, time_prov_factory));
}

live_model_base::live_model_base(const utility::configuration& config, std::function<void(const api_status&)> error_cb,
    trace_logger_factory_t* trace_factory, data_transport_factory_t* t_factory, model_factory_t* m_factory,
    sender_factory_t* s_factory, time_provider_factory_t* time_prov_factory)
{
  _pimpl = std::unique_ptr<live_model_impl>(new live_model_impl(
      config, std::move(error_cb), trace_factory, t_factory, m_factory, s_factory, time_prov_factory));
}

live_model_base::live_model_base(live_model_base&& other) noexcept
{
  std::swap(_pimpl, other._pimpl);
  _initialized = other._initialized;
}

template <typename ErrCntxt>
live_model_base::live_model_base(const utility::configuration& config, error_fn_t<ErrCntxt> fn, ErrCntxt* err_context,
    trace_logger_factory_t* trace_factory, data_transport_factory_t* t_factory, model_factory_t* m_factory,
    sender_factory_t* s_factory, time_provider_factory_t* time_prov_factory)
    : live_model_base(config, std::bind(fn, std::placeholders::_1, err_context), trace_factory, t_factory, m_factory,
          s_factory, time_prov_factory)
{
}

live_model_base::~live_model_base() = default;

live_model_base& live_model_base::operator=(live_model_base&& other) noexcept
{
  std::swap(_pimpl, other._pimpl);
  _initialized = other._initialized;
  return *this;
}

int live_model_base::init(api_status* status)
{
  if (_initialized) { return error_code::success; }

  const auto err_code = _pimpl->init(status);
  if (err_code == error_code::success) { _initialized = true; }

  return err_code;
}

int live_model_base::report_action_taken(const char* event_id, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_action_taken(event_id, status);
}

int live_model_base::report_action_taken(const char* primary_id, const char* secondary_id, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_action_taken(primary_id, secondary_id, status);
}

int live_model_base::refresh_model(api_status* status)
{
  INIT_CHECK();
  return _pimpl->refresh_model(status);
}

}  // namespace reinforcement_learning
