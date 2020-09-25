#pragma once
#include "learning_mode.h"
#include "logger/logger_facade.h"
#include "model_mgmt.h"
#include "model_mgmt/data_callback_fn.h"
#include "model_mgmt/model_downloader.h"
#include "utility/periodic_background_proc.h"

#include "factory_resolver.h"
#include "utility/watchdog.h"

#include <atomic>
#include <memory>

namespace reinforcement_learning
{
  class safe_vw_factory;
  class safe_vw;
  class ranking_response;
  class api_status;

  class live_model_impl {
  public:
    using error_fn = void(*)( const api_status&, void* user_context );

    int init(api_status* status);

    int choose_rank(const char* event_id, const char* context, unsigned int flags, ranking_response& response, api_status* status);
    //here the event_id is auto-generated
    int choose_rank(const char* context, unsigned int flags, ranking_response& response, api_status* status);
    int request_decision(const char* context_json, unsigned int flags, decision_response& resp, api_status* status);
    int request_slates_decision(const char* event_id, const char* context_json, unsigned int flags, slates_response& resp, api_status* status = nullptr);
    int request_slates_decision(const char* context_json, unsigned int flags, slates_response& resp, api_status* status = nullptr);

    int report_action_taken(const char* event_id, api_status* status);

    int report_outcome(const char* event_id, const char* outcome_data, api_status* status);
    int report_outcome(const char* event_id, float reward, api_status* status);
    int report_outcome(const char* event_id, float outcome, int idx, api_status* status= nullptr);
    int report_outcome(const char* event_id, float outcome, const char *idx, api_status* status= nullptr);
    int report_outcome(const char* event_id, const char* outcome, int idx, api_status* status= nullptr);
    int report_outcome(const char* event_id, const char* outcome, const char *idx, api_status* status= nullptr);

    int refresh_model(api_status* status);

    explicit live_model_impl(
      const utility::configuration& config,
      error_fn fn,
      void* err_context,
      trace_logger_factory_t* trace_factory,
      data_transport_factory_t* t_factory,
      model_factory_t* m_factory,
      sender_factory_t* sender_factory,
      time_provider_factory_t* time_provider_factory);

    live_model_impl(const live_model_impl&) = delete;
    live_model_impl(live_model_impl&&) = delete;
    live_model_impl& operator=(const live_model_impl&) = delete;
    live_model_impl& operator=(live_model_impl&&) = delete;

  private:
    // Internal implementation methods
    int init_model(api_status* status);
    int init_model_mgmt(api_status* status);
    int init_loggers(api_status* status);
    int init_trace(api_status* status);
    static void _handle_model_update(const model_management::model_data& data, live_model_impl* ctxt);
    void handle_model_update(const model_management::model_data& data);
    int explore_only(const char* event_id, const char* context, ranking_response& response, api_status* status) const;
    int explore_exploit(const char* event_id, const char* context, ranking_response& response, api_status* status) const;
    template<typename D>
    int report_outcome_internal(const char* event_id, D outcome, api_status* status);
    template<typename D, typename I>
    int report_outcome_internal(const char* event_id, D outcome, I index, api_status* status);

  private:
    // Internal implementation state
    std::atomic_bool _model_ready{false};
    float _initial_epsilon = 0.2f;
    utility::configuration _configuration;
    error_callback_fn _error_cb;
    model_management::data_callback_fn _data_cb;
    utility::watchdog _watchdog;
    learning_mode _learning_mode;

    trace_logger_factory_t* _trace_factory;
    data_transport_factory_t* _t_factory;
    model_factory_t* _m_factory;
    sender_factory_t* _sender_factory;
    time_provider_factory_t* _time_provider_factory;

    std::unique_ptr<model_management::i_data_transport> _transport{nullptr};
    std::unique_ptr<model_management::i_model> _model{nullptr};
    std::unique_ptr<logger::cb_logger_facade> _ranking_logger{nullptr};
    std::unique_ptr<logger::observation_logger_facade> _outcome_logger{nullptr};
    std::unique_ptr<logger::ccb_logger_facade> _decision_logger{nullptr};
    std::unique_ptr<logger::slates_logger_facade> _slates_logger{nullptr};
    std::unique_ptr<model_management::model_downloader> _model_download{nullptr};
    std::unique_ptr<i_trace> _trace_logger{nullptr};

    std::unique_ptr<utility::periodic_background_proc<model_management::model_downloader>> _bg_model_proc;
    uint64_t _seed_shift;
  };

  template <typename D>
  int live_model_impl::report_outcome_internal(const char* event_id, D outcome, api_status* status) {
    // Clear previous errors if any
    api_status::try_clear(status);

    // Send the outcome event to the backend
    RETURN_IF_FAIL(_outcome_logger->log(event_id, outcome, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog.has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }

  template <typename D, typename I>
  int live_model_impl::report_outcome_internal(const char* event_id, D outcome, I index, api_status* status) {
    // Clear previous errors if any
    api_status::try_clear(status);

    // Send the outcome event to the backend
    RETURN_IF_FAIL(_outcome_logger->log(event_id, index, outcome, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog.has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }
}
