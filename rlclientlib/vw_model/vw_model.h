#pragma once
#include "model_mgmt.h"
#include "safe_vw.h"
#include "../utility/versioned_object_pool.h"

namespace reinforcement_learning {
  class i_trace;
}

namespace reinforcement_learning { namespace model_management {
  class vw_model : public i_model {
  public:
    vw_model(i_trace* trace_logger);
    vw_model(i_trace* trace_logger, std::string& initial_command_line);
    int update(const model_data& data, bool& model_ready, api_status* status = nullptr) override;
    int choose_rank(uint64_t rnd_seed, const char* features, std::vector<int>& action_ids, std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) override;
    int request_decision(std::vector<const char*>& event_ids, const char* features, std::vector<std::vector<size_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) override;
  private:
    using vw_ptr = std::shared_ptr<safe_vw>;
    using pooled_vw = utility::pooled_object_guard<safe_vw, safe_vw_factory>;
    utility::versioned_object_pool<safe_vw, safe_vw_factory> _vw_pool;
    i_trace* _trace_logger;
  };
}}
