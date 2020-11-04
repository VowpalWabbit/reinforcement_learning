#include "http_authorization.h"

#include <vector>
#include <sstream>

#define OPENSSL_API_COMPAT 0x0908
#include <openssl/hmac.h>

#include "error_callback_fn.h"
#include "err_constants.h"
#include "http_client.h"
#include "trace_logger.h"

using namespace std::chrono;

namespace reinforcement_learning {
  http_authorization::http_authorization(
    const std::string& host,
    const std::string& key_name,
    const std::string& key,
    const std::string& name,
    i_trace* trace)
    : _eventhub_host(host)
    , _shared_access_key_name(key_name)
    , _shared_access_key(key)
    , _eventhub_name(name)
    , _valid_until(0)
    , _trace(trace) {
  }

  int http_authorization::init(const i_http_client* client, api_status* status) {
    return check_authorization_validity_generate_if_needed(client, status);
  }

  int http_authorization::get(const i_http_client* client, std::string& authorization, api_status* status) {
    RETURN_IF_FAIL(check_authorization_validity_generate_if_needed(client, status));
    std::lock_guard<std::mutex> lock(_mutex);
    authorization = _authorization;
    return error_code::success;
  }

  int http_authorization::check_authorization_validity_generate_if_needed(const i_http_client* client, api_status* status) {
    const auto now = duration_cast<std::chrono::seconds>(system_clock::now().time_since_epoch());
    std::lock_guard<std::mutex> lock(_mutex);
    // re-create authorization token if needed
    if (now.count() > _valid_until - 60 * 15) {
      RETURN_IF_FAIL(generate_authorization_string(
        client, now, _shared_access_key, _shared_access_key_name, _eventhub_host, _eventhub_name,
        _authorization, _valid_until, status, _trace));
    }
    return error_code::success;
  }

  int http_authorization::generate_authorization_string(
    const i_http_client* client,
    std::chrono::seconds now,
    const std::string& shared_access_key,
    const std::string& shared_access_key_name,
    const std::string& eventhub_host,
    const std::string& eventhub_name,
    std::string& authorization_string /* out */,
    long long& valid_until /* out */,
    api_status* status,
    i_trace* trace) {

    // Update saved valid_until value.
    valid_until = now.count() + 60 * 60 * 24 * 7; // 1 week

    // construct "sr"
    std::ostringstream resource_stream;
    resource_stream << "https://" << eventhub_host << "/" << eventhub_name;

    // encode(resource_stream)
    const auto encoded_uri = client->encode(resource_stream.str());

    // construct data to be signed
    std::ostringstream data_stream;
    data_stream << encoded_uri << "\n" << valid_until;
    std::string data = data_stream.str();

    // compute HMAC of data
    std::vector<unsigned char> digest(EVP_MAX_MD_SIZE);
    unsigned int digest_len;
    // https://www.openssl.org/docs/man1.0.2/crypto/hmac.html
    if (!HMAC(EVP_sha256(), shared_access_key.c_str(), (int)shared_access_key.length(),
      (const unsigned char*)data.c_str(), (int)data.length(), &digest[0], &digest_len)) {
      TRACE_ERROR(trace, "Failed to generate SAS hash");
      RETURN_ERROR_ARG(trace, status, eventhub_generate_SAS_hash, "Failed to generate SAS hash");
    }
    digest.resize(digest_len);

    // encode digest (base64 + url encoding)
    const auto encoded_digest = client->encode(digest);

    // construct SAS
    std::ostringstream authorization_stream;
    authorization_stream
      << "SharedAccessSignature sr=" << encoded_uri
      << "&sig=" << encoded_digest
      << "&se=" << valid_until
      << "&skn=" << shared_access_key_name;
    authorization_string = authorization_stream.str();

    return error_code::success;
  }
}
