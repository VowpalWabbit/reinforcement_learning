#include "empty_data_transport.h"
#include "api_status.h"
#include "factory_resolver.h"

namespace u = reinforcement_learning::utility;

namespace reinforcement_learning { namespace model_management {
  int empty_data_transport::get_data(model_data& ret, api_status* status) {
      ret.increment_refresh_count();

      return error_code::success;
  }
}}
