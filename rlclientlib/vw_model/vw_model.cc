#include "vw_model.h"

#include "constants.h"
#include "err_constants.h"
#include "object_factory.h"
#include "ranking_response.h"
#include "str_util.h"

#include <fstream>

namespace reinforcement_learning
{
namespace model_management
{
vw_model::vw_model(i_trace* trace_logger, const utility::configuration& config)
    : _audit(config.get_bool(name::AUDIT_ENABLED, false))
    , _audit_output_path(config.get(name::AUDIT_OUTPUT_PATH, value::DEFAULT_AUDIT_OUTPUT_PATH))
    , _initial_command_line(std::string(config.get(name::MODEL_VW_INITIAL_COMMAND_LINE,
                                "--cb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A")) +
          (_audit ? " --audit" : ""))
    , _vw_pool(safe_vw_factory(_initial_command_line, _dedup_cache),
          config.get_int(name::VW_POOL_INIT_SIZE, value::DEFAULT_VW_POOL_INIT_SIZE), trace_logger)
    , _trace_logger(trace_logger)
{
}

int vw_model::update(const model_data& data, bool& model_ready, api_status* status)
{
  try
  {
    TRACE_INFO(_trace_logger, utility::concat("Received new model data. With size ", data.data_sz()));

    if (data.data_sz() > 0)
    {
      std::string cmd_line = add_optional_audit_flag(_quiet_commandline_options);

      std::unique_ptr<safe_vw> init_vw(new safe_vw(data.data(), data.data_sz(), cmd_line, _dedup_cache));
      if (init_vw->is_CB_to_CCB_model_upgrade(_initial_command_line))
      {
        cmd_line = add_optional_audit_flag(_upgrade_to_CCB_vw_commandline_options);
      }

      safe_vw_factory factory(data, cmd_line, _dedup_cache);
      std::unique_ptr<safe_vw> test_vw(factory());
      if (test_vw->is_compatible(_initial_command_line))
      {
        // safe_vw_factory will create a copy of the model data to use for vw object construction.
        _vw_pool.update_factory(factory);
        model_ready = true;
      }
      else
      {
        RETURN_ERROR_LS(_trace_logger, status, model_update_error)
            << "Received model is incompatible with initial configuration " << _initial_command_line;
      }
    }
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_update_error) << e.what();
  }
  catch (...)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_update_error) << "Unknown error";
  }

  return error_code::success;
}

int vw_model::load_action(uint64_t action_id, std::string action_str, api_status* status)
{
  std::lock_guard<std::mutex> lock(_mutex);
  auto vw = _vw_pool.get_or_create();
  vw->load_action(action_id, action_str);
  return error_code::success;
}

int vw_model::choose_rank(const char* event_id, uint64_t rnd_seed, string_view features, std::vector<int>& action_ids,
    std::vector<float>& action_pdf, std::string& model_version, api_status* status)
{
  try
  {
    auto vw = _vw_pool.get_or_create();

    // Get a ranked list of action_ids and corresponding pdf
    std::lock_guard<std::mutex> lock(_mutex);
    vw->rank(features, action_ids, action_pdf);

    if (_audit) { write_audit_log(event_id, vw->get_audit_data()); }

    model_version = vw->id();

    return error_code::success;
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
  }
  catch (...)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unknown error";
  }
}

int vw_model::choose_rank_multistep(const char* event_id, uint64_t rnd_seed, string_view features,
    const episode_history& history, std::vector<int>& action_ids, std::vector<float>& action_pdf,
    std::string& model_version, api_status* status)
{
  std::lock_guard<std::mutex> lock(_mutex);
  return choose_rank(event_id, rnd_seed, features, action_ids, action_pdf, model_version, status);
}

int vw_model::choose_continuous_action(
    string_view features, float& action, float& pdf_value, std::string& model_version, api_status* status)
{
  try
  {
    auto vw = _vw_pool.get_or_create();

    vw->choose_continuous_action(features, action, pdf_value);

    model_version = vw->id();

    return error_code::success;
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
  }
  catch (...)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unknown error";
  }
}

int vw_model::request_decision(const std::vector<const char*>& event_ids, string_view features,
    std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs,
    std::string& model_version, api_status* status)
{
  try
  {
    auto vw = _vw_pool.get_or_create();

    // Get a ranked list of action_ids and corresponding pdf
    vw->rank_decisions(event_ids, features, actions_ids, action_pdfs);

    model_version = vw->id();

    return error_code::success;
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
  }
  catch (...)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unknown error";
  }
}

int vw_model::request_multi_slot_decision(const char* event_id, const std::vector<std::string>& slot_ids,
    string_view features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs,
    std::string& model_version, api_status* status)
{
  try
  {
    auto vw = _vw_pool.get_or_create();

    // Get a ranked list of action_ids and corresponding pdf
    vw->rank_multi_slot_decisions(event_id, slot_ids, features, actions_ids, action_pdfs);

    if (_audit) { write_audit_log(event_id, vw->get_audit_data()); }

    model_version = vw->id();

    return error_code::success;
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
  }
  catch (...)
  {
    RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unknown error";
  }
}

std::string vw_model::add_optional_audit_flag(const std::string& command_line) const
{
  if (_audit) { return command_line + " --audit"; }

  return command_line;
}

void vw_model::write_audit_log(const char* event_id, string_view audit_buffer) const
{
  if (event_id != nullptr && !audit_buffer.empty())
  {
    // remove any non-alphanumeric characters from the output name
    std::string filename(event_id);
    auto it = std::remove_if(filename.begin(), filename.end(), [](char const& c) { return !isalnum(c); });
    filename.erase(it, filename.end());

    std::ostringstream filepath;
    filepath << _audit_output_path << reinforcement_learning::PATH_DELIMITER << filename;

    std::ofstream auditFile;
    auditFile.open(filepath.str(), std::ofstream::out | std::ofstream::trunc);
    auditFile.write(audit_buffer.data(), audit_buffer.size());
    auditFile.close();
  }
}

model_type_t vw_model::model_type() const { return safe_vw::get_model_type(_initial_command_line); }

}  // namespace model_management
}  // namespace reinforcement_learning
