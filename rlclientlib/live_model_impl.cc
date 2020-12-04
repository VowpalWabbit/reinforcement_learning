#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

#include "utility/context_helper.h"
#include "sender.h"
#include "api_status.h"
#include "configuration.h"
#include "error_callback_fn.h"
#include "ranking_response.h"
#include "live_model_impl.h"
#include "err_constants.h"
#include "constants.h"
#include "vw_model/safe_vw.h"
#include "trace_logger.h"
#include "explore_internal.h"
#include "hash.h"
#include "factory_resolver.h"
#include "logger/preamble_sender.h"
#include "sampling.h"

#include <cstring>

// Some namespace changes for more concise code
namespace e = exploration;
using namespace std;

namespace reinforcement_learning {
  // Some namespace changes for more concise code
  namespace m = model_management;
  namespace u = utility;
  namespace l = logger;

  // Some typdefs for more concise code
  using vw_ptr = std::shared_ptr<safe_vw>;
  using pooled_vw = utility::pooled_object_guard<safe_vw, safe_vw_factory>;

  int check_null_or_empty(const char* arg1, const char* arg2, i_trace* trace, api_status* status);
  int check_null_or_empty(const char* arg1, i_trace* trace, api_status* status);
  int reset_action_order(ranking_response& response);
  void autogenerate_missing_uuids(const std::map<size_t, std::string>& found_ids, std::vector<std::string>& complete_ids, uint64_t seed_shift);

  void default_error_callback(const api_status& status, void* watchdog_context) {
    auto watchdog = static_cast<utility::watchdog*>(watchdog_context);
    watchdog->set_unhandled_background_error(true);
  }

  int live_model_impl::init(api_status* status) {
    RETURN_IF_FAIL(init_trace(status));
    RETURN_IF_FAIL(init_model(status));
    RETURN_IF_FAIL(init_model_mgmt(status));
    RETURN_IF_FAIL(init_loggers(status));

    if (_protocol_version == 1 &&
      _configuration.get(name::INTERACTION_CONTENT_ENCODING, value::CONTENT_ENCODING_IDENTITY) != value::CONTENT_ENCODING_IDENTITY) {
      RETURN_ERROR_LS(_trace_logger.get(), status, content_encoding_error);
    }

    _initial_epsilon = _configuration.get_float(name::INITIAL_EPSILON, 0.2f);
    const char* app_id = _configuration.get(name::APP_ID, "");
    _seed_shift = uniform_hash(app_id, strlen(app_id), 0);
    return error_code::success;
  }

  int live_model_impl::choose_rank(const char* event_id, const char* context, unsigned int flags, ranking_response& response,
    api_status* status) {
    response.clear();
    //clear previous errors if any
    api_status::try_clear(status);

    //check arguments
    RETURN_IF_FAIL(check_null_or_empty(event_id, context, _trace_logger.get(), status));
    if (!_model_ready) {
      RETURN_IF_FAIL(explore_only(event_id, context, response, status));
      response.set_model_id("N/A");
    }
    else {
      RETURN_IF_FAIL(explore_exploit(event_id, context, response, status));
    }
    response.set_event_id(event_id);

    if (_learning_mode == LOGGINGONLY)
    {
      // Reset the ranked action order before logging
      RETURN_IF_FAIL(reset_action_order(response));
    }

    RETURN_IF_FAIL(_interaction_logger->log(context, flags, response, status, _learning_mode));

    if (_learning_mode == APPRENTICE)
    {
      // Reset the ranked action order after logging
      RETURN_IF_FAIL(reset_action_order(response));
    }

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog.has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }

  //here the event_id is auto-generated
  int live_model_impl::choose_rank(const char* context, unsigned int flags, ranking_response& response, api_status* status) {
    const auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    return choose_rank(uuid.c_str(), context, flags, response,
      status);
  }

  int live_model_impl::request_continuous_action(const char* event_id, const char* context, unsigned int flags, continuous_action_response& response, api_status* status)
  {
    response.clear();
    //clear previous errors if any
    api_status::try_clear(status);

    RETURN_IF_FAIL(check_null_or_empty(event_id, context, _trace_logger.get(), status));
    
    float action;
    float pdf_value;
    std::string model_version;

    RETURN_IF_FAIL(_model->choose_continuous_action(context, action, pdf_value, model_version, status));
    RETURN_IF_FAIL(populate_response(action, pdf_value, std::string(event_id), std::string(model_version), response, _trace_logger.get(), status));
    RETURN_IF_FAIL(_interaction_logger->log_continuous_action(context, flags, response, status));
    
    if (_watchdog.has_background_error_been_reported())
    {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }
    
  int live_model_impl::request_continuous_action(const char* context, unsigned int flags, continuous_action_response& response, api_status* status)
  {
    const auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    return request_continuous_action(uuid.c_str(), context, flags, response, status);
  }

  int live_model_impl::request_decision(const char* context_json, unsigned int flags, decision_response& resp, api_status* status)
  {
    if (_learning_mode == APPRENTICE || _learning_mode == LOGGINGONLY) {
      // Apprentice mode and LoggingOnly mode are not supported here at this moment
      return error_code::not_supported;
    }

    resp.clear();
    //clear previous errors if any
    api_status::try_clear(status);

    //check arguments
    RETURN_IF_FAIL(check_null_or_empty(context_json, _trace_logger.get(), status));

    utility::ContextInfo context_info;
    RETURN_IF_FAIL(utility::get_context_info(context_json, context_info, _trace_logger.get(), status));

    // Ensure multi comes before slots, this is a current limitation of the parser.
    if(context_info.slots.size() < 1 || context_info.actions.size() < 1 || context_info.slots[0].first < context_info.actions[0].first) {
      RETURN_ERROR_LS(_trace_logger.get(), status, json_parse_error) << "There must be both a _multi field and _slots, and _multi must come first.";
    }

    std::vector<std::vector<uint32_t>> actions_ids;
    std::vector<std::vector<float>> actions_pdfs;
    std::string model_version;

    size_t num_decisions = context_info.slots.size();

    std::vector<std::string> event_ids_str(num_decisions);
    std::vector<const char*> event_ids(num_decisions, nullptr);
    std::map<size_t, std::string> found_ids;
    RETURN_IF_FAIL(utility::get_event_ids(context_json, found_ids, _trace_logger.get(), status));
    
    autogenerate_missing_uuids(found_ids, event_ids_str, _seed_shift);
    
    for (int i = 0; i < event_ids.size(); i++)
    {
      event_ids[i] = event_ids_str[i].c_str();
    }

    // This will behave correctly both before a model is loaded and after. Prior to a model being loaded it operates in explore only mode.
    RETURN_IF_FAIL(_model->request_decision(event_ids, context_json, actions_ids, actions_pdfs, model_version, status));
    RETURN_IF_FAIL(populate_response(actions_ids, actions_pdfs, event_ids, std::string(model_version), resp, _trace_logger.get(), status));
    RETURN_IF_FAIL(_interaction_logger->log_decisions(event_ids, context_json, flags, actions_ids, actions_pdfs, model_version, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog.has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }

    return error_code::success;
  }

  int live_model_impl::request_multi_slot_decision_impl(const char *event_id, const char * context_json, std::vector<std::string>& slot_ids, std::vector<std::vector<uint32_t>>& action_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status)
  {
    if (_learning_mode == APPRENTICE || _learning_mode == LOGGINGONLY) {
      // Apprentice mode and LoggingOnly mode are not supported here at this moment
      return error_code::not_supported;
    }
    //clear previous errors if any
    api_status::try_clear(status);

    //check arguments
    RETURN_IF_FAIL(check_null_or_empty(event_id, _trace_logger.get(), status));
    RETURN_IF_FAIL(check_null_or_empty(context_json, _trace_logger.get(), status));

    utility::ContextInfo context_info;
    RETURN_IF_FAIL(utility::get_context_info(context_json, context_info, _trace_logger.get(), status));

    // Ensure multi comes before slots, this is a current limitation of the parser.
    if (context_info.slots.size() < 1 || context_info.actions.size() < 1 || context_info.slots[0].first < context_info.actions[0].first) {
      RETURN_ERROR_LS(_trace_logger.get(), status, json_parse_error) << "There must be both a _multi field and _slots, and _multi must come first.";
    }

    slot_ids.resize(context_info.slots.size());
    std::map<size_t, std::string> found_ids;
    RETURN_IF_FAIL(utility::get_slot_ids(context_json, context_info.slots, found_ids, _trace_logger.get(), status));
    autogenerate_missing_uuids(found_ids, slot_ids, _seed_shift);

    RETURN_IF_FAIL(_model->request_multi_slot_decision(event_id, slot_ids, context_json, action_ids, action_pdfs, model_version, status));
    return error_code::success;
  }

  int live_model_impl::request_multi_slot_decision(const char * context_json, unsigned int flags, multi_slot_response& resp, api_status* status)
  {
    const auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    return request_multi_slot_decision(uuid.c_str(), context_json, flags, resp, status);
  }

  int live_model_impl::request_multi_slot_decision(const char * event_id, const char * context_json, unsigned int flags, multi_slot_response& resp, api_status* status)
  {
    resp.clear();

    std::vector<std::string> slot_ids;
    std::vector<std::vector<uint32_t>> action_ids;
    std::vector<std::vector<float>> action_pdfs;
    std::string model_version;

    RETURN_IF_FAIL(live_model_impl::request_multi_slot_decision_impl(event_id, context_json, slot_ids, action_ids, action_pdfs, model_version, status));

    RETURN_IF_FAIL(populate_multi_slot_response(action_ids, action_pdfs, std::string(event_id), std::string(model_version), slot_ids, resp, _trace_logger.get(), status));
    RETURN_IF_FAIL(_interaction_logger->log_decision(event_id, context_json, flags, action_ids, action_pdfs, model_version, slot_ids, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog.has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }
    return error_code::success;
  }

  int live_model_impl::request_multi_slot_decision(const char * context_json, unsigned int flags, multi_slot_response_detailed& resp, api_status* status)
  {
    const auto uuid = boost::uuids::to_string(boost::uuids::random_generator()());
    return request_multi_slot_decision(uuid.c_str(), context_json, flags, resp, status);
  }
  
  int live_model_impl::request_multi_slot_decision(const char * event_id, const char * context_json, unsigned int flags, multi_slot_response_detailed& resp, api_status* status)
  {
    resp.clear();

    std::vector<std::string> slot_ids;
    std::vector<std::vector<uint32_t>> action_ids;
    std::vector<std::vector<float>> action_pdfs;
    std::string model_version;

    RETURN_IF_FAIL(live_model_impl::request_multi_slot_decision_impl(event_id, context_json, slot_ids, action_ids, action_pdfs, model_version, status));

    //set the size of buffer in response to match the number of slots
    resp.resize(slot_ids.size());

    RETURN_IF_FAIL(populate_multi_slot_response_detailed(action_ids, action_pdfs, std::string(event_id), std::string(model_version), slot_ids, resp, _trace_logger.get(), status));
    RETURN_IF_FAIL(_interaction_logger->log_decision(event_id, context_json, flags, action_ids, action_pdfs, model_version, slot_ids, status));

    // Check watchdog for any background errors. Do this at the end of function so that the work is still done.
    if (_watchdog.has_background_error_been_reported()) {
      RETURN_ERROR_LS(_trace_logger.get(), status, unhandled_background_error_occurred);
    }
    return error_code::success;
  }

  int live_model_impl::report_action_taken(const char* event_id, api_status* status) {
    // Clear previous errors if any
    api_status::try_clear(status);
    // Send the outcome event to the backend
    return _outcome_logger->report_action_taken(event_id, status);
  }

  int live_model_impl::report_outcome(const char* event_id, const char* outcome, api_status* status) {
    // Check arguments
    RETURN_IF_FAIL(check_null_or_empty(event_id, outcome, _trace_logger.get(), status));
    return report_outcome_internal(event_id, outcome, status);
  }

  int live_model_impl::report_outcome(const char* event_id, float outcome, api_status* status) {
    // Check arguments
    RETURN_IF_FAIL(check_null_or_empty(event_id, _trace_logger.get(), status));
    return report_outcome_internal(event_id, outcome, status);
  }

 int live_model_impl::report_outcome(const char* primary_id, int secondary_id, const char* outcome, api_status* status) {
    // Check arguments
    RETURN_IF_FAIL(check_null_or_empty(primary_id, outcome, _trace_logger.get(), status));
    return report_outcome_internal(primary_id, secondary_id, outcome, status);
  }

  int live_model_impl::report_outcome(const char* primary_id, int secondary_id, float outcome, api_status* status) {
    // Check arguments
    RETURN_IF_FAIL(check_null_or_empty(primary_id, _trace_logger.get(), status));
    return report_outcome_internal(primary_id, secondary_id, outcome, status);
  }

 int live_model_impl::report_outcome(const char* primary_id, const char* secondary_id, const char* outcome, api_status* status) {
    // Check arguments
    RETURN_IF_FAIL(check_null_or_empty(primary_id, outcome, _trace_logger.get(), status));
    RETURN_IF_FAIL(check_null_or_empty(secondary_id, _trace_logger.get(), status));
    return report_outcome_internal(primary_id, secondary_id, outcome, status);
  }

  int live_model_impl::report_outcome(const char* primary_id, const char* secondary_id, float outcome, api_status* status) {
    // Check arguments
    RETURN_IF_FAIL(check_null_or_empty(primary_id, _trace_logger.get(), status));
    RETURN_IF_FAIL(check_null_or_empty(secondary_id, _trace_logger.get(), status));
    return report_outcome_internal(primary_id, secondary_id, outcome, status);
  }

  int live_model_impl::refresh_model(api_status* status) {

    if (_bg_model_proc) {
      RETURN_ERROR_LS(_trace_logger.get(), status, model_update_error)
        << "Cannot manually refresh model when backround polling is enabled";
    }

    model_management::model_data md;
    RETURN_IF_FAIL(_transport->get_data(md, status));

    bool model_ready = false;
    RETURN_IF_FAIL(_model->update(md, model_ready, status));

    _model_ready = model_ready;

    return error_code::success;
  }

  live_model_impl::live_model_impl(
    const utility::configuration& config,
    const error_fn fn,
    void* err_context,
    trace_logger_factory_t* trace_factory,
    data_transport_factory_t* t_factory,
    model_factory_t* m_factory,
    sender_factory_t* sender_factory,
    time_provider_factory_t* time_provider_factory
  )
    : _configuration(config),
    _error_cb(fn, err_context),
    _data_cb(_handle_model_update, this),
    _watchdog(&_error_cb),
    _trace_factory(trace_factory),
    _t_factory{ t_factory },
    _m_factory{ m_factory },
    _sender_factory{ sender_factory },
    _time_provider_factory{ time_provider_factory },
    _protocol_version(_configuration.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION))
  {
    // If there is no user supplied error callback, supply a default one that does nothing but report unhandled background errors.
    if (fn == nullptr) {
      _error_cb.set(&default_error_callback, &_watchdog);
    }

    if (_configuration.get_bool(name::MODEL_BACKGROUND_REFRESH, value::DEFAULT_MODEL_BACKGROUND_REFRESH)) {
      _bg_model_proc.reset(new utility::periodic_background_proc<model_management::model_downloader>(config.get_int(name::MODEL_REFRESH_INTERVAL_MS, 60 * 1000), _watchdog, "Model downloader", &_error_cb));
    }

    _learning_mode = learning::to_learning_mode(_configuration.get(name::LEARNING_MODE, value::LEARNING_MODE_ONLINE));
  }

  int live_model_impl::init_trace(api_status* status) {
    const auto trace_impl = _configuration.get(name::TRACE_LOG_IMPLEMENTATION, value::NULL_TRACE_LOGGER);
    i_trace* plogger;
    RETURN_IF_FAIL(_trace_factory->create(&plogger, trace_impl, _configuration, nullptr, status));
    _trace_logger.reset(plogger);
    TRACE_INFO(_trace_logger, "API Tracing initialized");
    _watchdog.set_trace_log(_trace_logger.get());
    return error_code::success;
  }

  int live_model_impl::init_model(api_status* status) {
    const auto model_impl = _configuration.get(name::MODEL_IMPLEMENTATION, value::VW);
    m::i_model* pmodel;
    RETURN_IF_FAIL(_m_factory->create(&pmodel, model_impl, _configuration, _trace_logger.get(), status));
    _model.reset(pmodel);
    return error_code::success;
  }

  int live_model_impl::init_loggers(api_status* status) {
    // Get the name of raw data (as opposed to message) sender for interactions.
    const auto* const ranking_sender_impl = _configuration.get(name::INTERACTION_SENDER_IMPLEMENTATION, value::get_default_interaction_sender());
    i_sender* ranking_data_sender;

    // Use the name to create an instance of raw data sender for interactions
    RETURN_IF_FAIL(_sender_factory->create(&ranking_data_sender, ranking_sender_impl, _configuration, &_error_cb, _trace_logger.get(), status));
    RETURN_IF_FAIL(ranking_data_sender->init(status));

    // Create a message sender that will prepend the message with a preamble and send the raw data using the
    // factory created raw data sender
    l::i_message_sender* ranking_msg_sender = new l::preamble_message_sender(ranking_data_sender);
    RETURN_IF_FAIL(ranking_msg_sender->init(status));

    // Get time provider factory and implementation
    const auto* const time_provider_impl = _configuration.get(name::TIME_PROVIDER_IMPLEMENTATION, value::get_default_time_provider());

    i_time_provider* logger_extensions_time_provider;
    RETURN_IF_FAIL(_time_provider_factory->create(&logger_extensions_time_provider, time_provider_impl, _configuration, _trace_logger.get(), status));

    //Create the logger extension
    _logger_extensions.reset(logger::i_logger_extensions::get_extensions(_configuration, logger_extensions_time_provider));

    i_time_provider* ranking_time_provider;
    RETURN_IF_FAIL(_time_provider_factory->create(&ranking_time_provider, time_provider_impl, _configuration, _trace_logger.get(), status));

    // Create a logger for interactions that will use msg sender to send interaction messages
    _interaction_logger.reset(new logger::interaction_logger_facade(_model->model_type(), _configuration, ranking_msg_sender, _watchdog, ranking_time_provider, *_logger_extensions.get(), &_error_cb));
    RETURN_IF_FAIL(_interaction_logger->init(status));

    // Get the name of raw data (as opposed to message) sender for observations.
    const auto* const outcome_sender_impl = _configuration.get(name::OBSERVATION_SENDER_IMPLEMENTATION, value::get_default_observation_sender());
    i_sender* outcome_sender;

    // Use the name to create an instance of raw data sender for observations
    RETURN_IF_FAIL(_sender_factory->create(&outcome_sender, outcome_sender_impl, _configuration, &_error_cb, _trace_logger.get(), status));
    RETURN_IF_FAIL(outcome_sender->init(status));

    // Create a message sender that will prepend the message with a preamble and send the raw data using the
    // factory created raw data sender
    l::i_message_sender* outcome_msg_sender = new l::preamble_message_sender(outcome_sender);
    RETURN_IF_FAIL(outcome_msg_sender->init(status));

    // Get time provider implementation
    i_time_provider* observation_time_provider;
    RETURN_IF_FAIL(_time_provider_factory->create(&observation_time_provider, time_provider_impl, _configuration, _trace_logger.get(), status));

    // Create a logger for interactions that will use msg sender to send interaction messages
    _outcome_logger.reset(new logger::observation_logger_facade(_configuration, outcome_msg_sender, _watchdog, observation_time_provider, &_error_cb));
    RETURN_IF_FAIL(_outcome_logger->init(status));

    return error_code::success;
  }

  void inline live_model_impl::_handle_model_update(const m::model_data& data, live_model_impl* ctxt) {
    ctxt->handle_model_update(data);
  }

  void live_model_impl::handle_model_update(const model_management::model_data& data) {
    if (data.refresh_count() == 0) {
      TRACE_INFO(_trace_logger, "Model was not updated since previous download");
      return;
    }

    api_status status;

    bool model_ready = false;

    if (_model->update(data, model_ready, &status) != error_code::success) {
      _error_cb.report_error(status);
      return;
    }
    _model_ready = model_ready;
  }

  int live_model_impl::explore_only(const char* event_id, const char* context, ranking_response& response,
    api_status* status) const {

    // Generate egreedy pdf
    utility::ContextInfo context_info;
    RETURN_IF_FAIL(utility::get_context_info(context, context_info, _trace_logger.get(), status));

    size_t action_count = context_info.actions.size();
    if(action_count < 1) {
        RETURN_ERROR_LS(_trace_logger.get(), status, json_no_actions_found) << "Context must have at least one action";
    }

    vector<float> pdf(action_count);
    // Generate a pdf with epsilon distributed between all action.
    // The top action gets the remaining (1 - epsilon)
    // Assume that the user's top choice for action is at index 0
    const auto top_action_id = 0;
    auto scode = e::generate_epsilon_greedy(_initial_epsilon, top_action_id, begin(pdf), end(pdf));
    if (S_EXPLORATION_OK != scode) {
      RETURN_ERROR_LS(_trace_logger.get(), status, exploration_error) << "Exploration error code: " << scode;
    }

    // The seed used is composed of uniform_hash(app_id) + uniform_hash(event_id)
    const uint64_t seed = uniform_hash(event_id, strlen(event_id), 0) + _seed_shift;

    // Pick a slot using the pdf. NOTE: sample_after_normalizing() can change the pdf
    uint32_t chosen_index;
    scode = e::sample_after_normalizing(seed, begin(pdf), end(pdf), chosen_index);

    if (S_EXPLORATION_OK != scode) {
      RETURN_ERROR_LS(_trace_logger.get(), status, exploration_error) << "Exploration error code: " << scode;
    }

    // NOTE: When there is no model, the rank
    // step was done by the user.  i.e. Actions are already in ranked order
    // If there were an action list it would be [0,1,2,3,4..].  The index
    // of the list matches the action_id.  There is no need to generate this
    // list of actions we can use the index into this list as a proxy for the
    // actual action_id.
    // i.e  chosen_index == action[chosen_index]
    // Why is this documented?  Because explore_exploit uses a model and we
    // cannot make the same assumption there.  (Bug was fixed)

    // Setup response with pdf from prediction and chosen action
    // Chosen action goes first.  First action gets swapped with chosen action
    for (size_t idx = 0; idx < pdf.size(); ++idx) {
      response.push_back(idx, pdf[idx]);
    }

    // Swap values in first position with values in chosen index
    scode = e::swap_chosen(begin(response), end(response), chosen_index);

    if (S_EXPLORATION_OK != scode) {
      RETURN_ERROR_LS(_trace_logger.get(), status, exploration_error) << "Exploration (Swap) error code: " << scode;
    }

    response.set_chosen_action_id(chosen_index);

    return error_code::success;
  }

  int live_model_impl::explore_exploit(const char* event_id, const char* context, ranking_response& response,
    api_status* status) const {
    // The seed used is composed of uniform_hash(app_id) + uniform_hash(event_id)
    const uint64_t seed = uniform_hash(event_id, strlen(event_id), 0) + _seed_shift;

    std::vector<int> action_ids;
    std::vector<float> action_pdf;
    std::string model_version;

    RETURN_IF_FAIL(_model->choose_rank(seed, context, action_ids, action_pdf, model_version, status));

    return sample_and_populate_response(seed, action_ids, action_pdf, std::move(model_version), response, _trace_logger.get(), status);
  }

  int live_model_impl::init_model_mgmt(api_status* status) {
    // Initialize transport for the model using transport factory
    const auto tranport_impl = _configuration.get(name::MODEL_SRC, value::get_default_data_transport());
    m::i_data_transport* ptransport;
    RETURN_IF_FAIL(_t_factory->create(&ptransport, tranport_impl, _configuration, status));
    // This class manages lifetime of transport
    this->_transport.reset(ptransport);

    if (_bg_model_proc) {
      // Initialize background process and start downloading models
      this->_model_download.reset(new m::model_downloader(ptransport, &_data_cb, _trace_logger.get()));
      return _bg_model_proc->init(_model_download.get(), status);
    }

    return refresh_model(status);
  }

  //helper: check if at least one of the arguments is null or empty
  int check_null_or_empty(const char* arg1, const char* arg2, i_trace* trace, api_status* status) {
    if (!arg1 || !arg2 || strlen(arg1) == 0 || strlen(arg2) == 0) {
      RETURN_ERROR_ARG(trace, status, invalid_argument, "one of the arguments passed to the ds is null or empty");
    }
    return error_code::success;
  }

  int check_null_or_empty(const char* arg1,  i_trace* trace, api_status* status) {
    if (!arg1 || strlen(arg1) == 0) {
      RETURN_ERROR_ARG(trace, status, invalid_argument, "one of the arguments passed to the ds is null or empty");
    }
    return error_code::success;
  }

  int reset_action_order(ranking_response& response) {
#ifdef __clang__
    std::vector<action_prob> tmp;
    std::copy(response.begin(), response.end(), std::back_inserter(tmp));
    std::sort(tmp.begin(), tmp.end(), [](const action_prob& a, const action_prob& b) {
      return a.action_id < b.action_id;
    }
    );
    std::copy(tmp.begin(), tmp.end(), response.begin());
#else
    std::sort(response.begin(), response.end(), [](const action_prob& a, const action_prob& b) {
      return a.action_id < b.action_id;
    }
    );
#endif
    response.set_chosen_action_id((*(response.begin())).action_id);

    return error_code::success;
  }

  void autogenerate_missing_uuids(const std::map<size_t, std::string>& found_ids, std::vector<std::string>& complete_ids, uint64_t seed_shift) {
    for (auto ids : found_ids)
    {
      complete_ids[ids.first] = ids.second;
    }

    for (int i = 0; i < complete_ids.size(); i++)
    {
      if (complete_ids[i].empty())
      {
        complete_ids[i] = boost::uuids::to_string(boost::uuids::random_generator()()) + std::to_string(seed_shift);
      }
    }    
  }
}
