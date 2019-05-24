#pragma once

namespace reinforcement_learning {  namespace name {
      const char *const  APP_ID                  = "appid";
      const char *const  MODEL_SRC               = "model.source";
      const char *const  MODEL_BLOB_URI          = "model.blob.uri";
      const char *const  MODEL_REFRESH_INTERVAL_MS  = "model.refreshintervalms";
      const char *const  MODEL_IMPLEMENTATION    = "model.implementation";       // VW vs other ML
      const char *const  MODEL_BACKGROUND_REFRESH = "model.backgroundrefresh";
      const char *const  VW_CMDLINE              = "vw.commandline";
      const char *const  INITIAL_EPSILON         = "initial_exploration.epsilon";
      const char *const  INTERACTION_EH_HOST     = "interaction.eventhub.host";
      const char *const  INTERACTION_EH_NAME     = "interaction.eventhub.name";
      const char *const  INTERACTION_EH_KEY_NAME = "interaction.eventhub.keyname";
      const char *const  INTERACTION_EH_KEY      = "interaction.eventhub.key";
      const char *const  INTERACTION_EH_TASKS_LIMIT = "interaction.eventhub.tasks_limit";
      const char *const  INTERACTION_EH_MAX_HTTP_RETRIES = "interaction.eventhub.max_http_retries";
      const char *const  OBSERVATION_EH_HOST     = "observation.eventhub.host";
      const char *const  OBSERVATION_EH_NAME     = "observation.eventhub.name";
      const char *const  OBSERVATION_EH_KEY_NAME = "observation.eventhub.keyname";
      const char *const  OBSERVATION_EH_KEY      = "observation.eventhub.key";
      const char *const  OBSERVATION_EH_TASKS_LIMIT = "observation.eventhub.tasks_limit";
      const char *const  OBSERVATION_EH_MAX_HTTP_RETRIES = "observation.eventhub.max_http_retries";
      const char *const  INTERACTION_SEND_HIGH_WATER_MARK     = "interaction.send.highwatermark";
      const char *const  INTERACTION_SEND_QUEUE_MAX_CAPACITY_KB    = "interaction.send.queue.maxcapacity.kb";
      const char *const  INTERACTION_SEND_BATCH_INTERVAL_MS   = "interaction.send.batchintervalms";
      const char *const  OBSERVATION_SEND_HIGH_WATER_MARK     = "observation.send.highwatermark";
      const char *const  OBSERVATION_SEND_QUEUE_MAX_CAPACITY_KB    = "observation.send.queue.maxcapacity.kb";
      const char *const  OBSERVATION_SEND_BATCH_INTERVAL_MS   = "observation.send.batchintervalms";
      const char *const  OBSERVATION_SENDER_IMPLEMENTATION    = "observation.sender.implementation";
      const char *const  INTERACTION_SENDER_IMPLEMENTATION    = "interaction.sender.implementation";
      const char *const  EH_TEST                 = "eventhub.mock";
      const char *const  TRACE_LOG_IMPLEMENTATION = "trace.logger.implementation";
      const char *const  QUEUE_MODE = "queue.mode";
      const char *const  INTERACTION_FILE_NAME = "interaction.file.name";
      const char *const  OBSERVATION_FILE_NAME = "observation.file.name";
      const char *const  TIME_PROVIDER_IMPLEMENTATION = "time_provider.implementation";
      const char *const  HTTP_CLIENT_DISABLE_CERT_VALIDATION  = "http.certvalidation.disable";
      const char *const  HTTP_CLIENT_TIMEOUT                  = "http.timeout"; // Timeout is in seconds, default is 30.
}}

namespace reinforcement_learning {  namespace value {
      const char *const AZURE_STORAGE_BLOB = "AZURE_STORAGE_BLOB";
      const char *const NO_MODEL_DATA = "NO_MODEL_DATA";
      const char *const VW                 = "VW";
      const char *const PASSTHROUGH_PDF_MODEL = "PASSTHROUGH_PDF";
      const char *const OBSERVATION_EH_SENDER = "OBSERVATION_EH_SENDER";
      const char *const INTERACTION_EH_SENDER = "INTERACTION_EH_SENDER";
      const char *const OBSERVATION_FILE_SENDER = "OBSERVATION_FILE_SENDER";
      const char *const INTERACTION_FILE_SENDER = "INTERACTION_FILE_SENDER";
      const char *const NULL_TRACE_LOGGER = "NULL_TRACE_LOGGER";
      const char *const CONSOLE_TRACE_LOGGER = "CONSOLE_TRACE_LOGGER";
      const char *const NULL_TIME_PROVIDER = "NULL_TIME_PROVIDER";
      const char *const CLOCK_TIME_PROVIDER = "CLOCK_TIME_PROVIDER";
      const bool DEFAULT_MODEL_BACKGROUND_REFRESH = true;
}}

