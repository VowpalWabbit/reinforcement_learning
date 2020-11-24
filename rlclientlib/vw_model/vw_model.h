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
  class vw_model : public i_model {
  public:
    vw_model(i_trace* trace_logger, const utility::configuration& config);

    int update(const model_data& data, bool& model_ready, api_status* status = nullptr) override;
    int choose_rank(uint64_t rnd_seed, const char* features, std::vector<int>& action_ids, std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) override;
    int choose_continuous_action(const char* features, float& action, float& pdf_value, std::string& model_version, api_status* status = nullptr) override;
    int request_decision(const std::vector<const char*>& event_ids, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) override;
    int request_multi_slot_decision(const char *event_id, const std::vector<std::string>& slot_ids, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) override;
    model_type_t model_type() const override;

  private:
    const std::string _initial_command_line;
	const std::string _upgrade_to_CCB_vw_commandline_options{ "--ccb_explore_adf --json --quiet" };

    using vw_ptr = std::shared_ptr<safe_vw>;
    using pooled_vw = utility::pooled_object_guard<safe_vw, safe_vw_factory>;
    utility::versioned_object_pool<safe_vw, safe_vw_factory> _vw_pool;
    i_trace* _trace_logger;
  };
}}
