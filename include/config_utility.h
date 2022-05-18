#pragma once
#include <string>
#include "vw/common/vwvis.h"

namespace reinforcement_learning {namespace utility {
  class configuration;
}
class VW_DLL_PUBLIC api_status;
class VW_DLL_PUBLIC i_trace;
}

namespace reinforcement_learning { namespace utility { namespace config {
  VW_DLL_PUBLIC std::string load_config_json();
  VW_DLL_PUBLIC int create_from_json(const std::string& config_json, configuration& cc, i_trace* trace = nullptr, api_status* = nullptr);
}}}
