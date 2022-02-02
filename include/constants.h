#pragma once

namespace reinforcement_learning {  namespace name {
      const char *const  APP_ID                  = "appid";
      const char *const  MODEL_SRC               = "model.source";
      const char *const  MODEL_BLOB_URI          = "model.blob.uri";
      const char* const  MODEL_HTTP_APPI_HOST = "model.http.api.host";
      const char *const  MODEL_REFRESH_INTERVAL_MS = "model.refreshintervalms";
      const char *const  MODEL_IMPLEMENTATION    = "model.implementation";       // VW vs other ML
      const char *const  MODEL_BACKGROUND_REFRESH = "model.backgroundrefresh";
      const char *const  MODEL_VW_INITIAL_COMMAND_LINE = "model.vw.initial_command_line";
      const char *const  VW_CMDLINE              = "vw.commandline";
      const char *const  VW_POOL_INIT_SIZE       = "vw.pool.init.size";
      const char *const  INITIAL_EPSILON         = "initial_exploration.epsilon";
      const char *const  LEARNING_MODE           = "rank.learning.mode";
      const char* const  PROTOCOL_VERSION             = "protocol.version";
      const char* const  HTTP_API_KEY            = "http.api.key";
      const char* const  HTTP_KEY_TYPE           = "http.key.type";

      // Episode
      const char *const EPISODE_EH_HOST     = "episode.eventhub.host";
      const char *const EPISODE_EH_NAME     = "episode.eventhub.name";
      const char *const EPISODE_EH_KEY_NAME = "episode.eventhub.keyname";
      const char *const EPISODE_EH_KEY      = "episode.eventhub.key";
      const char *const EPISODE_EH_TASKS_LIMIT = "episode.eventhub.tasks_limit";
      const char *const EPISODE_EH_MAX_HTTP_RETRIES = "episode.eventhub.max_http_retries";
      const char *const EPISODE_SENDER_IMPLEMENTATION    = "episode.sender.implementation";

      // Interaction
      const char *const  INTERACTION_EH_HOST     = "interaction.eventhub.host";
      const char *const  INTERACTION_EH_NAME     = "interaction.eventhub.name";
      const char *const  INTERACTION_EH_KEY_NAME = "interaction.eventhub.keyname";
      const char *const  INTERACTION_EH_KEY      = "interaction.eventhub.key";
      const char *const  INTERACTION_EH_TASKS_LIMIT = "interaction.eventhub.tasks_limit";
      const char *const  INTERACTION_EH_MAX_HTTP_RETRIES = "interaction.eventhub.max_http_retries";
      const char *const  INTERACTION_SEND_HIGH_WATER_MARK     = "interaction.send.highwatermark";
      const char *const  INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB    = "interaction.send.queue.maxcapacity.kb";
      const char *const  INTERACTION_SEND_BATCH_INTERVAL_MS   = "interaction.send.batchintervalms";
      const char *const  INTERACTION_SENDER_IMPLEMENTATION    = "interaction.sender.implementation";
      const char *const  INTERACTION_USE_COMPRESSION = "interaction.send.use_compression";
      const char *const  INTERACTION_USE_DEDUP = "interaction.send.use_dedup";
      const char *const  INTERACTION_QUEUE_MODE = "interaction.queue.mode";
      const char *const  INTERACTION_HTTP_API_HOST = "interaction.http.api.host";
      const char *const  INTERACTION_APIM_TASKS_LIMIT = "interaction.apim.tasks_limit";
      const char *const  INTERACTION_APIM_MAX_HTTP_RETRIES = "interaction.apim.max_http_retries";
      const char *const  INTERACTION_SUBSAMPLE_RATE = "interaction.subsample.rate";

      // Observation
      const char *const  OBSERVATION_EH_HOST     = "observation.eventhub.host";
      const char *const  OBSERVATION_EH_NAME     = "observation.eventhub.name";
      const char *const  OBSERVATION_EH_KEY_NAME = "observation.eventhub.keyname";
      const char *const  OBSERVATION_EH_KEY      = "observation.eventhub.key";
      const char *const  OBSERVATION_EH_TASKS_LIMIT = "observation.eventhub.tasks_limit";
      const char *const  OBSERVATION_EH_MAX_HTTP_RETRIES = "observation.eventhub.max_http_retries";
      const char *const  OBSERVATION_SEND_HIGH_WATER_MARK     = "observation.send.highwatermark";
      const char *const  OBSERVATION_SEND_QUEUE_MAX_CAPACITY_KB    = "observation.send.queue.maxcapacity.kb";
      const char *const  OBSERVATION_SEND_BATCH_INTERVAL_MS   = "observation.send.batchintervalms";
      const char *const  OBSERVATION_SENDER_IMPLEMENTATION    = "observation.sender.implementation";
      const char *const  OBSERVATION_USE_COMPRESSION = "observation.send.use_compression";
      const char *const  OBSERVATION_QUEUE_MODE = "observation.queue.mode";
      const char *const  OBSERVATION_HTTP_API_HOST = "observation.http.api.host";
      const char *const  OBSERVATION_APIM_TASKS_LIMIT = "observation.apim.tasks_limit";
      const char *const  OBSERVATION_APIM_MAX_HTTP_RETRIES = "observation.apim.max_http_retries";
      const char *const  OBSERVATION_SUBSAMPLE_RATE = "observation.subsample.rate";

      //global sender properties
      const char *const SEND_HIGH_WATER_MARK        = "send.highwatermark";
      const char *const SEND_QUEUE_MAX_CAPACITY_KB  = "send.queue.maxcapacity.kb";
      const char *const SEND_BATCH_INTERVAL_MS      = "send.batchintervalms";
      const char *const USE_COMPRESSION             = "send.use_compression";
      const char *const USE_DEDUP                   = "send.use_dedup";
      const char *const QUEUE_MODE                  = "queue.mode";
      const char *const SUBSAMPLE_RATE              = "subsample.rate";

      const char *const  EH_TEST                 = "eventhub.mock";
      const char *const  TRACE_LOG_IMPLEMENTATION = "trace.logger.implementation";
      const char *const  EPISODE_FILE_NAME = "episode.file.name";
      const char *const  INTERACTION_FILE_NAME = "interaction.file.name";
      const char *const  OBSERVATION_FILE_NAME = "observation.file.name";
      const char *const  TIME_PROVIDER_IMPLEMENTATION = "time_provider.implementation";
      const char *const  HTTP_CLIENT_DISABLE_CERT_VALIDATION  = "http.certvalidation.disable";
      const char *const  HTTP_CLIENT_TIMEOUT                  = "http.timeout"; // Timeout is in seconds, default is 30.
      const char *const  MODEL_FILE_NAME                      = "model_file_loader.file_name";
      const char *const  MODEL_FILE_MUST_EXIST                = "model_file_loader.file_must_exist";

      const char *const ZSTD_COMPRESSION_LEVEL = "zstd.compression_level";
}}

namespace reinforcement_learning {  namespace value {
      const char *const AZURE_STORAGE_BLOB = "AZURE_STORAGE_BLOB";
      const char *const NO_MODEL_DATA = "NO_MODEL_DATA";
      const char *const FILE_MODEL_DATA = "FILE_MODEL_DATA";
      const char *const VW                 = "VW";
      const char *const PASSTHROUGH_PDF_MODEL = "PASSTHROUGH_PDF";
      const char *const EPISODE_EH_SENDER = "EPISODE_EH_SENDER";
      const char *const OBSERVATION_EH_SENDER = "OBSERVATION_EH_SENDER";
      const char *const INTERACTION_EH_SENDER = "INTERACTION_EH_SENDER";
      const char *const EPISODE_FILE_SENDER = "EPISODE_FILE_SENDER";
      const char *const OBSERVATION_FILE_SENDER = "OBSERVATION_FILE_SENDER";
      const char *const INTERACTION_FILE_SENDER = "INTERACTION_FILE_SENDER";
      const char* const OBSERVATION_HTTP_API_SENDER = "OBSERVATION_HTTP_API_SENDER";
      const char* const INTERACTION_HTTP_API_SENDER = "INTERACTION_HTTP_API_SENDER";
      const char *const NULL_TRACE_LOGGER = "NULL_TRACE_LOGGER";
      const char *const CONSOLE_TRACE_LOGGER = "CONSOLE_TRACE_LOGGER";
      const char *const NULL_TIME_PROVIDER = "NULL_TIME_PROVIDER";
      const char *const CLOCK_TIME_PROVIDER = "CLOCK_TIME_PROVIDER";
      const char *const LEARNING_MODE_ONLINE = "ONLINE";
      const char *const LEARNING_MODE_APPRENTICE = "APPRENTICE";
      const char *const LEARNING_MODE_LOGGINGONLY = "LOGGINGONLY";
      const char *const CONTENT_ENCODING_IDENTITY = "IDENTITY";
      const char *const CONTENT_ENCODING_DEDUP = "DEDUP";

      const char *const QUEUE_MODE_DROP = "DROP";
      const char *const QUEUE_MODE_BLOCK = "BLOCK";

      const bool DEFAULT_MODEL_BACKGROUND_REFRESH = true;
      const int DEFAULT_VW_POOL_INIT_SIZE = 4;
      const int DEFAULT_PROTOCOL_VERSION = 1;

      const char *get_default_episode_sender();
      const char *get_default_observation_sender();
      const char *get_default_interaction_sender();
      const char *get_default_data_transport();
      const char *get_default_time_provider();
}}

namespace reinforcement_learning {  namespace constants {
      // subsampling uses drop_pass of -1 to avoid collision with the queue's pruning function
      constexpr int SUBSAMPLE_RATE_DROP_PASS = -1;
}}
