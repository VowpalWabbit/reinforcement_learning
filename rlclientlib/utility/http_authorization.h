#pragma once

#include "api_status.h"
#include "moving_queue.h"
#include "sender.h"
#include "error_callback_fn.h"

#include "utility/http_client.h"

#include <pplx/pplxtasks.h>

#include <queue>
#include <chrono>
#include <memory>

namespace reinforcement_learning {
  class i_trace;

  // The eventhub_client send string data in POST requests to an HTTP endpoint.
  // It handles authorization headers specific for the Azure event hubs.
  class http_authorization {
  public:
    http_authorization(const std::string& host, const std::string& key_name,
      const std::string& key, const std::string& name, i_trace* trace);
    ~http_authorization() = default;
    
    int init(api_status* status);
    int get(std::string& authorization, api_status* status);

  private:
    int check_authorization_validity_generate_if_needed(api_status* status);

    static int generate_authorization_string(
      std::chrono::seconds now,
      const std::string& shared_access_key,
      const std::string& shared_access_key_name,
      const std::string& eventhub_host,
      const std::string& eventhub_name,
      std::string& authorization_string /* out */,
      long long& valid_until /* out */,
      api_status* status,
      i_trace* trace);

    // cannot be copied or assigned
    http_authorization(const http_authorization&) = delete;
    http_authorization(http_authorization&&) = delete;
    http_authorization& operator=(const http_authorization&) = delete;
    http_authorization& operator=(http_authorization&&) = delete;

  private:
    const std::string _eventhub_host; //e.g. "ingest-x2bw4dlnkv63q.servicebus.windows.net"
    const std::string _shared_access_key_name; //e.g. "RootManageSharedAccessKey"
    const std::string _shared_access_key;
    //e.g. Check https://docs.microsoft.com/en-us/azure/event-hubs/event-hubs-authentication-and-security-model-overview
    const std::string _eventhub_name; //e.g. "interaction"

    std::string _authorization;
    long long _valid_until; //in seconds
    std::mutex _mutex;
    i_trace* _trace;
  };
}
