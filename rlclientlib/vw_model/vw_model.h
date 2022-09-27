#pragma once
#include "../utility/versioned_object_pool.h"
#include "model_mgmt.h"
#include "multistep.h"
#include "safe_vw.h"
#include "trace_logger.h"

namespace reinforcement_learning
{
namespace utility
{
class configuration;
}
}  // namespace reinforcement_learning

namespace reinforcement_learning
{
namespace model_management
{
class vw_model : public i_model
{
  std::string add_optional_audit_flag(const std::string& command_line) const;
  void write_audit_log(const char* event_id, string_view audit_buffer) const;

public:
  vw_model(i_trace* trace_logger, const utility::configuration& config);

  int update(const model_data& data, bool& model_ready, api_status* status = nullptr) override;
  int choose_rank(const char* event_id, uint64_t rnd_seed, string_view features, std::vector<int>& action_ids,
      std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) override;
  int choose_continuous_action(string_view features, float& action, float& pdf_value, std::string& model_version,
      api_status* status = nullptr) override;
  int request_decision(const std::vector<const char*>& event_ids, string_view features,
      std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs,
      std::string& model_version, api_status* status = nullptr) override;
  int request_multi_slot_decision(const char* event_id, const std::vector<std::string>& slot_ids, string_view features,
      std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs,
      std::string& model_version, api_status* status = nullptr) override;
  int choose_rank_multistep(const char* event_id, uint64_t rnd_seed, string_view features,
      const episode_history& history, std::vector<int>& action_ids, std::vector<float>& action_pdf,
      std::string& model_version, api_status* status = nullptr) override;
  model_type_t model_type() const override;

private:
  const bool _audit;
  const std::string _audit_output_path;
  const std::string _initial_command_line;
  const std::string _quiet_commandline_options{"--json --quiet"};
  const std::string _upgrade_to_CCB_vw_commandline_options{"--ccb_explore_adf --json --quiet"};
  utility::versioned_object_pool<safe_vw> _vw_pool;
  i_trace* _trace_logger;
};
}  // namespace model_management
}  // namespace reinforcement_learning
