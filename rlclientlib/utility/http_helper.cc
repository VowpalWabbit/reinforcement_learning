#include "http_helper.h"
#include "constants.h"

#include <chrono>

namespace reinforcement_learning {
  namespace utility {
    web::http::client::http_client_config get_http_config(const utility::configuration &cfg) {
      web::http::client::http_client_config config;

      config.set_validate_certificates(cfg.get_bool(name::HTTP_CLIENT_DISABLE_CERT_VALIDATION, false));
      int timeout = cfg.get_int(name::HTTP_CLIENT_TIMEOUT, 30);
	    //Valid values are 1-30
      if (timeout < 1 || timeout > 30)
		    timeout = 30;
      config.set_timeout(std::chrono::seconds(timeout));
      return config;
    }
  }
}
