#pragma once
#include "configuration.h"

#include <cpprest/http_client.h>

namespace reinforcement_learning
{
namespace utility
{
web::http::client::http_client_config get_http_config(const utility::configuration& cfg);

}
}  // namespace reinforcement_learning