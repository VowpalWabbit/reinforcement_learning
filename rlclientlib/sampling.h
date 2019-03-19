#pragma once
#include "model_mgmt.h"
#include <vector>

namespace reinforcement_learning {
  class i_trace;
}

namespace reinforcement_learning {
    int sample_and_populate_response(uint64_t rnd_seed, std::vector<int>& action_ids, std::vector<float>& pdf, std::string&& model_id, ranking_response& response, i_trace* trace_logger, api_status* status);
}