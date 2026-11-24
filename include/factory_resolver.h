#pragma once
#include "azure_credentials_provider.h"
#include "oauth_callback_fn.h"
#include "object_factory.h"

#include <chrono>
#include <vector>
namespace reinforcement_learning
{
namespace utility
{
class configuration;
class watchdog;
}  // namespace utility

// Forward declarations
namespace model_management
{
class i_data_transport;
class i_model;
}  // namespace model_management
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
 * @brief Factory to create Azure credential providers used to retrieve Azure OAuth tokens.
 * Advanced extension point: Register another implementation of i_oauth_credentials_provider to
 * provide the mechanism used when providing Azure OAuth token to the API implementation.
 *
 * @remark azure_cred_provider_factory_t provides the necessary Azure credential implementation used
 * by the library to retrieve Azure OAuth tokens. The following Azure credential providers are
 * provided by the library:
 *
 * - azure_credentials_provider<azure::identity::client_secret_credential> - Provides Azure OAuth token using client
 * secret.
 * - azure_credentials_provider<azure::identity::managed_identity> - Provides Azure OAuth token using managed identity.
 * - azure_credentials_provider<azure::identity::azure_cli> - Provides Azure OAuth token using Azure CLI.
 * - azure_credentials_provider<azure::identity::default_credential> - Provides Azure OAuth token using default
 * credential.
 *
 * The library provides a default implementation of azure_credentials_provider<azure::identity::default_credential>.
 *
 * The following configuration keys are used to configure the Azure credential provider:
 *
 * - "azure.oauth.credential.type" - The type of Azure credential provider to use. The following values are supported:
 *    - "CLIENT_SECRET" - Use azure_credentials_provider<azure::identity::client_secret_credential> to provide Azure
 * OAuth token.
 *    - "MANAGED_IDENTITY" - Use azure_credentials_provider<azure::identity::managed_identity> to provide Azure OAuth
 * token.
 *    - "AZURECLI" - Use azure_credentials_provider<azure::identity::azure_cli> to provide Azure OAuth token.
 *    - "DEFAULT_CREDENTIAL" - Use azure_credentials_provider<azure::identity::default_credential> to provide Azure
 * OAuth token.
 *
 * The following configuration keys are used to configure the Azure credential provider:
 *
 * - CLIENT_SECRET:
 *    - "azure.oauth.credential.clientid"
 *    - "azure.oauth.credential.tenantid"
 *    - "azure.oauth.credential.clientsecret"
 *
 * - MANAGED_IDENTITY:
 *    - "azure.oauth.credential.clientid"
 *
 * - AZURECLI:
 *    - "azure.oauth.credential.tenantid"
 *
 * To enable Azure OAuth token authentication, the following configuration keys may be set:
 *
 * "model.source": "HTTP_MODEL_DATA_OAUTH_AZ"
 * "interaction.sender.implementation": "INTERACTION_HTTP_API_SENDER_OAUTH_AZ"
 * "observation.sender.implementation": "OBSERVATION_HTTP_API_SENDER_OAUTH_AZ"
 * "episode.sender.implementation": "EPISODE_HTTP_API_SENDER_OAUTH_AZ"
 *
 * For a custom implementation, register your implementations using the factory. Or, @see
 * register_default_factories_callback if you don't want to use the factory.
 */
using azure_cred_provider_factory_t =
    utility::object_factory<i_oauth_credentials_provider, const utility::configuration&>;
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
using data_transport_factory_t =
    utility::object_factory<model_management::i_data_transport, const utility::configuration&>;
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

extern data_transport_factory_t& data_transport_factory;
extern model_factory_t& model_factory;
extern sender_factory_t& sender_factory;
extern trace_logger_factory_t& trace_logger_factory;
extern time_provider_factory_t& time_provider_factory;
extern azure_cred_provider_factory_t& azure_cred_provider_factory;

// For proper static intialization
// Check https://en.wikibooks.org/wiki/More_C++_Idioms/Nifty_Counter for explanation
struct factory_initializer
{
  factory_initializer();
  ~factory_initializer();

private:
  static void register_default_factories();
};
// Every translation unit gets a factory_initializer
// only one translation unit will initialize it
static factory_initializer _init;

/**
 * @brief Register default factories with an authentication callback.
 * @remark This function can be called to register factories for
 * retrieving auth tokens. This is useful when control over retrieving
 * tokens is needed and can be provided by the application.
 *
 * To enable this feature, the application must call this function prior to
 * calling API methods that retrieve sender/receiver objects that require it.
 * The configuration file should define some or all of the following keys:
 *
 * "model.source": "HTTP_MODEL_DATA_OAUTH"
 * "episode.sender.implementation": "EPISODE_HTTP_API_SENDER_OAUTH"
 * "interaction.sender.implementation": "INTERACTION_HTTP_API_SENDER_OAUTH"
 * "observation.sender.implementation": "OBSERVATION_HTTP_API_SENDER_OAUTH"
 *
 * NOTE: The defaults for the above keys are as follows:
 *
 * -- If USE_AZURE_FACTORIES is NOT defined as part of the build:
 *
 * "model.source": "NO_MODEL_DATA"
 * "episode.sender.implementation": "EPISODE_FILE_SENDER"
 * "interaction.sender.implementation": "INTERACTION_FILE_SENDER"
 * "observation.sender.implementation": "OBSERVATION_FILE_SENDER"
 * "time_provider.implementation": "CLOCK_TIME_PROVIDER"
 *
 * -- If USE_AZURE_FACTORIES is defined as part of the build:
 *
 * "model.source": "AZURE_STORAGE_BLOB"
 * "episode.sender.implementation": "EPISODE_EH_SENDER"
 * "interaction.sender.implementation": "INTERACTION_EH_SENDER"
 * "observation.sender.implementation": "OBSERVATION_EH_SENDER"
 * "time_provider.implementation": "NULL_TIME_PROVIDER"
 *
 * See the rl_sim example application to see how to use this feature.
 */
void register_default_factories_callback(oauth_callback_t& callback);
}  // namespace reinforcement_learning
