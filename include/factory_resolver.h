#pragma once
#include "object_factory.h"
#include "vw/common/vwvis.h"

namespace reinforcement_learning  {
  namespace utility {
    class configuration;
    class watchdog;
  }

  // Forward declarations
  namespace model_management {
    class i_data_transport;
    class i_model;
  }
  class i_sender;
  class i_time_provider;
  class error_callback_fn;

  /**
  * @brief Factory to create trace loggers used to trace events in the library.
  * Advanced extension point:  Register another implementation of i_trace to
  * provide the mechanism used when logging internal events in the API implementation.
  */
  using trace_logger_factory_t = utility::object_factory<i_trace, const utility::configuration&>;
  /**
  * @brief Factory to create model used in inference.
  * Advanced extension point:  Register another implementation of i_model to
  * provide hydraded model given updated model data. This model is then used
  * in inference.
  */
  using model_factory_t = utility::object_factory<model_management::i_model, const utility::configuration&>;
  /**
   * @brief Factory to create transport for model data.
   * Advanced extension point:  Register another implementation of i_data_transport to
   * provide updated model data used to hydrate inference model.
   */
  using data_transport_factory_t = utility::object_factory<model_management::i_data_transport, const utility::configuration&>;
  /**
   * @brief Factory to create loggers used to record the interactions and observations.
   * Advanced extension point:  Register another implementation of i_logger to
   * provide the mechanism used when logging interaction and observation events.
   */
  using sender_factory_t = utility::object_factory<i_sender, const utility::configuration&, error_callback_fn*>;
  /**
   * @brief Factory to create time provider to set the client timestamp in messages.
   * Advanced extension point:  Register another implementation of i_time_provider to
   * provide the mechanism used to set timestamps
   */
  using time_provider_factory_t = utility::object_factory<i_time_provider, const utility::configuration&>;

  VW_DLL_PUBLIC extern data_transport_factory_t& data_transport_factory;
  VW_DLL_PUBLIC extern model_factory_t& model_factory;
  VW_DLL_PUBLIC extern sender_factory_t& sender_factory;
  VW_DLL_PUBLIC extern trace_logger_factory_t& trace_logger_factory;
  VW_DLL_PUBLIC extern time_provider_factory_t& time_provider_factory;

  // For proper static intialization
  // Check https://en.wikibooks.org/wiki/More_C++_Idioms/Nifty_Counter for explanation
  struct factory_initializer {
    VW_DLL_PUBLIC factory_initializer();
    VW_DLL_PUBLIC ~factory_initializer();
    private:
    static void register_default_factories();
  };
  // Every translation unit gets a factory_initializer
  // only one translation unit will initialize it
  static factory_initializer _init;
}
