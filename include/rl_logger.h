/**
 * @brief RL Contextual bandits logging API definition.
 *
 * @file rl_logger.h
 * @author Alexey Taymanov et al
 * @date 2020-02-24
 */
#pragma once
#include "err_constants.h"
#include "factory_resolver.h"

#include <memory>

namespace reinforcement_learning {

  //// Forward declarations ////////
         //
  class ranking_response;         //
  class decision_response;        //
  class api_status;               //
                                  //
  namespace utility {             //
    class configuration;          //
  }                               //
  //////////////////////////////////
  class rl_logger_impl;
  // Reinforcement learning client
  /**
   * @brief Interface class for the logging API.
   *
   * - (1) Instantiate and Initialize
   * - (2) log() to log contextual bandints evend ([event_id], context, response, outcome)
   */
  class rl_logger {
  public:
    /**
     * @brief Error callback function.
     * When rl_logger is constructed, a background error callback and a
     * context (void*) is registered. If there is an error in the background thread,
     * error callback will get invoked with api_status and the context (void*).
     *
     * NOTE: Error callback will get invoked in a background thread.
     */
    using error_fn = void(*)(const api_status&, void*);

    /**
     * @brief Construct a new logger object.
     *
     * @param config Name-Value based configuration
     * @param fn Error callback for handling errors in background thread
     * @param err_context Context passed back during Error callback
     * @param t_factory Transport factory.  The default transport factory is initialized with a
     *                  REST based transport that gets data from an Azure storage account
     * @param sender_factory Sender factory.  The default factory provides two senders, one for
     *                       interaction and the other for observation which logs to Event Hub.
     * @param time_provider_factory Time provider factory. The default factory provides time provider
     *                        returning gmt_now from std::chrono::system_clock::now().
     */
    rl_logger(
      const utility::configuration& config,
      error_fn fn = nullptr,
      void* err_context = nullptr,
      trace_logger_factory_t* trace_factory = &trace_logger_factory,
      sender_factory_t* s_factory = &sender_factory,
      time_provider_factory_t* time_prov_factory = &time_provider_factory);

    /**
     * @brief Initialize logger library.
     * Initialize the library and start the background threads used for
     * error handling and sending actions and outcomes to the online trainer
     * @param status  Optional field with detailed string description if there is an error
     * @return int Return error code.  This will also be returned in the api_status object
     */
    int init(api_status* status = nullptr);

    /**
     * @brief Log contextual bandit interaction
     * @param context_json Contains action, action features and context features in json format
     * @param resp Ranking response contains the chosen action, probability distribution used for sampling actions and ranked actions
     * @param status  Optional field with detailed string description if there is an error
     * @return int Return error code.  This will also be returned in the api_status object
     */
    int log(const char * context_json, const ranking_response& resp, api_status* status = nullptr);

    /**
     * @brief Log conditional contextual bandit interaction
     * @param context_json Contains action, action features and context features in json format
     * @param resp Decision response contains slots with slot id, chosen action, probability distribution used for sampling actions and ranked actions
     * @param status  Optional field with detailed string description if there is an error
     * @return int Return error code.  This will also be returned in the api_status object
     */
    int log(const char * context_json, const decision_response& resp, api_status* status = nullptr);

    /**
     * @brief Log contextual bandit or contextual conditional bandit reward
     * @param event_id EventId for CB / SlotId for CCB
     * @param outcome Outcome as JSON
     * @param status  Optional field with detailed string description if there is an error
     * @return int Return error code.  This will also be returned in the api_status object
     */
    int log(const char * event_id, const char* outcome, api_status* status = nullptr);

    /**
     * @brief Log contextual bandit or contextual conditional bandit reward
     * @param event_id EventId for CB / SlotId for CCB
     * @param outcome Outcome as float
     * @param status  Optional field with detailed string description if there is an error
     * @return int Return error code.  This will also be returned in the api_status object
     */
    int log(const char * event_id, float outcome, api_status* status = nullptr);

    template<typename ErrCntxt>
    using error_fn_t = void(*)(const api_status&, ErrCntxt*);

    /**
     * @brief Construct a new logger object.
     *
     * @tparam ErrCntxt Context type used in error callback.
     * @param config Name-Value based configuration
     * @param fn Error callback for handling errors in background thread
     * @param err_context Context passed back during Error callback
     * @param t_factory Transport factory.  The default transport factory is initialized with a
     *                  REST based transport that gets data from an Azure storage account
     * @param sender_factory Sender factory.  The default factory provides two senders, one for
     *                       interaction and the other for observation which logs to Event Hub.
     * @param time_provider_factory Time provider factory. The default factory provides time provider
     *                        returning gmt_now from std::chrono::system_clock::now().
     */
    template<typename ErrCntxt>
    rl_logger(
      const utility::configuration& config,
      error_fn_t<ErrCntxt> fn = nullptr,
      ErrCntxt* err_context = nullptr,
      trace_logger_factory_t* trace_factory = &trace_logger_factory,
      sender_factory_t* s_factory = &sender_factory,
      time_provider_factory_t* time_prov_factory = &time_provider_factory);

    /**
     * @brief Move constructor for logger object.
     */
    rl_logger(rl_logger&& other);

    /**
     * @brief Move assignment operator swaps implementation.
     */
    rl_logger& operator=(rl_logger&& other);
   
    rl_logger(const rl_logger&) = delete;       //! Prevent accidental copy, since destructor will deallocate the implementation
    rl_logger& operator=(rl_logger&) = delete;  //! Prevent accidental copy, since destructor will deallocate the implementation

    ~rl_logger();
  private:
    std::unique_ptr<rl_logger_impl> _pimpl;  //! The actual implementation details are forwarded to this object (PIMPL pattern)
    bool _initialized = false;                //! Guard to ensure that logger is properly initialized. i.e. init() was called and successfully initialized.
  };

  /**
   * @brief Construct a new logger object.
   *
   * @tparam ErrCntxt Context type used in error callback.
   * @param config Name-Value based configuration
   * @param fn Error callback for handling errors in background thread
   * @param err_context Context passed back during Error callback
   * @param t_factory Transport factory.  The default transport factory is initialized with a
   *                  REST based transport that gets data from an Azure storage account
   * @param sender_factory Sender factory.  The default factory provides two senders, one for
   *                       interaction and the other for observations which logs to Event Hub.
   * @param time_provider_factory Time provider factory. The default factory provides time provider
   *                        returning gmt_now from std::chrono::system_clock::now().
   */
  template<typename ErrCntxt>
  rl_logger::rl_logger(
    const utility::configuration& config,
    error_fn_t<ErrCntxt> fn,
    ErrCntxt* err_context,
    trace_logger_factory_t* trace_factory,
    sender_factory_t* s_factory,
    time_provider_factory_t* time_prov_factory)
    : rl_logger(config, (error_fn)(fn), (void*)(err_context), trace_factory, s_factory, time_prov_factory)
  {}
}
