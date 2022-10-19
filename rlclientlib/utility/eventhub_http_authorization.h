#pragma once

#include "api_status.h"
#include "configuration.h"
#include "internal_constants.h"

#include <cpprest/http_headers.h>

#include <chrono>
#include <mutex>
#include <string>

using namespace web::http;

namespace reinforcement_learning
{
class i_trace;

class eventhub_http_authorization
{
public:
  eventhub_http_authorization() = default;
  ~eventhub_http_authorization() = default;

  int init(const utility::configuration& config, api_status* status, i_trace* trace);
  int insert_authorization_header(http_headers& headers, api_status* status);
  int insert_authorization_header(http_headers& headers, api_status* status, i_trace* trace)
  {
    return insert_authorization_header(headers, status);
  }

  // cannot be copied or assigned
  eventhub_http_authorization(const eventhub_http_authorization&) = delete;
  eventhub_http_authorization(eventhub_http_authorization&&) = delete;
  eventhub_http_authorization& operator=(const eventhub_http_authorization&) = delete;
  eventhub_http_authorization& operator=(eventhub_http_authorization&&) = delete;

private:
  int check_authorization_validity_generate_if_needed(api_status* status);
  int get_authorization_token(std::string& authorization, api_status* status);

  static int generate_authorization_string(std::chrono::seconds now, const std::string& shared_access_key,
      const std::string& shared_access_key_name, const std::string& eventhub_host, const std::string& eventhub_name,
      std::string& authorization_string /* out */, long long& valid_until /* out */, api_status* status,
      i_trace* trace);

private:
  std::string _eventhub_host;           // e.g. "ingest-x2bw4dlnkv63q.servicebus.windows.net"
  std::string _shared_access_key_name;  // e.g. "RootManageSharedAccessKey"
  std::string _shared_access_key;
  // e.g. Check https://docs.microsoft.com/en-us/azure/event-hubs/event-hubs-authentication-and-security-model-overview
  std::string _eventhub_name;  // e.g. "interaction"

  std::string _authorization;
  long long _valid_until;  // in seconds
  std::mutex _mutex;
  i_trace* _trace;
};
}  // namespace reinforcement_learning
