#pragma once
#include "model_mgmt.h"
#include "safe_vw.h"
#include "../utility/versioned_object_pool.h"

namespace reinforcement_learning {
  class i_trace;
}

namespace reinforcement_learning { namespace model_management {
  class pdf_model : public i_model {
  public:
    pdf_model(i_trace* trace_logger);
    int update(const model_data& data, api_status* status = nullptr) override;
    int choose_rank(uint64_t rnd_seed, const char* features, ranking_response& response, api_status* status = nullptr) override;
  private:
	std::unique_ptr<safe_vw> _vw;
    i_trace* _trace_logger;
  };
}}
