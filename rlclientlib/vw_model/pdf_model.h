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
    int update(const model_data& data, bool& model_ready, api_status* status = nullptr) override;
    int choose_rank(uint64_t rnd_seed, const char* features, std::vector<int>& action_ids, std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) override;
    int request_decision(const char* features, std::vector<std::vector<int>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) override;
  private:
    std::unique_ptr<safe_vw> _vw;
    i_trace* _trace_logger;
    // TODO Should we provide some mechanism for the user to inject the model ID?
    // model_version = ??
    const std::string _model_version = "fake_version";
  };
}}
