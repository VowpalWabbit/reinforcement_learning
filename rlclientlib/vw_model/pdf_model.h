#pragma once
#include "model_mgmt.h"
#include "safe_vw.h"
#include "../utility/versioned_object_pool.h"

namespace reinforcement_learning {
  class i_trace;
  namespace utility {
    class configuration;
  }
}

namespace reinforcement_learning { namespace model_management {
  class pdf_model : public i_model {
  public:
    pdf_model(i_trace* trace_logger, const utility::configuration& config);
    int update(const model_data& data, bool& model_ready, api_status* status = nullptr) override;
    int choose_rank(uint64_t rnd_seed, string_view features, std::vector<int>& action_ids, std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) override;
    int choose_continuous_action(string_view features, float& action, float& pdf_value, std::string& model_version, api_status* status = nullptr) override;
    int request_decision(const std::vector<string_view>& event_ids, string_view features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) override;
    int request_multi_slot_decision(string_view event_id, const std::vector<std::string>& slot_ids, string_view features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) override;
    int choose_rank_multistep(uint64_t rnd_seed, string_view features, const episode_history& history, std::vector<int>& action_ids, std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) override;
    model_type_t model_type() const override;
  private:
    std::unique_ptr<safe_vw> _vw;
    i_trace* _trace_logger;
    // TODO Should we provide some mechanism for the user to inject the model ID?
    // model_version = ??
    const std::string _model_version = "fake_version";
  };
}}
