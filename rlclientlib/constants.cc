#include "constants.h"

namespace reinforcement_learning
{
namespace value
{

#ifdef USE_AZURE_FACTORIES
const char* const DEFAULT_EPISODE_SENDER = EPISODE_EH_SENDER;
const char* const DEFAULT_OBSERVATION_SENDER = OBSERVATION_EH_SENDER;
const char* const DEFAULT_INTERACTION_SENDER = INTERACTION_EH_SENDER;
const char* const DEFAULT_DATA_TRANSPORT = AZURE_STORAGE_BLOB;
const char* const DEFAULT_TIME_PROVIDER = NULL_TIME_PROVIDER;
#else
const char* const DEFAULT_EPISODE_SENDER = EPISODE_FILE_SENDER;
const char* const DEFAULT_OBSERVATION_SENDER = OBSERVATION_FILE_SENDER;
const char* const DEFAULT_INTERACTION_SENDER = INTERACTION_FILE_SENDER;
const char* const DEFAULT_DATA_TRANSPORT = NO_MODEL_DATA;
const char* const DEFAULT_TIME_PROVIDER = CLOCK_TIME_PROVIDER;
#endif

const char* get_default_episode_sender() { return DEFAULT_EPISODE_SENDER; }

const char* get_default_observation_sender() { return DEFAULT_OBSERVATION_SENDER; }

const char* get_default_interaction_sender() { return DEFAULT_INTERACTION_SENDER; }

const char* get_default_data_transport() { return DEFAULT_DATA_TRANSPORT; }

const char* get_default_time_provider() { return DEFAULT_TIME_PROVIDER; }
}  // namespace value
}  // namespace reinforcement_learning
