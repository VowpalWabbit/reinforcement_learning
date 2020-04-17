#include "constants.h"
#include "vw_model.h"
#include "err_constants.h"
#include "object_factory.h"
#include "ranking_response.h"
#include "trace_logger.h"
#include "str_util.h"

namespace reinforcement_learning { namespace model_management {

  vw_model::vw_model(i_trace* trace_logger, const utility::configuration& config)
    : _initial_command_line(config.get(name::MODEL_VW_INITIAL_COMMAND_LINE, "--cb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A"))
    , _vw_pool(new safe_vw_factory(_initial_command_line), config.get_int(name::VW_POOL_INIT_SIZE, value::DEFAULT_VW_POOL_INIT_SIZE))
    , _trace_logger(trace_logger) {
  }

  int vw_model::update(const model_data& data, bool& model_ready, api_status* status) {
    try {
      TRACE_INFO(_trace_logger, utility::concat("Received new model data. With size ", data.data_sz()));

      if (data.data_sz() > 0)
      {
        std::unique_ptr<safe_vw_factory> factory(new safe_vw_factory(std::move(data)));
        std::unique_ptr<safe_vw> test_vw((*factory)());
        if (test_vw->is_compatible(_initial_command_line)) {
          // safe_vw_factory will create a copy of the model data to use for vw object construction.
          _vw_pool.update_factory(factory.release());
          model_ready = true;
        }
        else {
          RETURN_ERROR_LS(_trace_logger, status, model_update_error) 
            << "Received model is incompatible with initial configuration " << _initial_command_line;
        }
      }
    }
    catch(const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, model_update_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(_trace_logger, status, model_update_error) << "Unknown error";
    }

    return error_code::success;
  }

  int vw_model::choose_rank(
    uint64_t rnd_seed,
    const char* features,
    std::vector<int>& action_ids,
    std::vector<float>& action_pdf,
    std::string& model_version,
    api_status* status) {
    try {
      pooled_vw vw(_vw_pool, _vw_pool.get_or_create());

      // Get a ranked list of action_ids and corresponding pdf
      vw->rank(features, action_ids, action_pdf);

      model_version = vw->id();

      return error_code::success;
    }
    catch ( const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unknown error";
    }
  }

  int vw_model::request_decision(const std::vector<const char*>& event_ids, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status)
  {
    try {
      pooled_vw vw(_vw_pool, _vw_pool.get_or_create());

      // Get a ranked list of action_ids and corresponding pdf
      vw->rank_decisions(event_ids, features, actions_ids, action_pdfs);

      model_version = vw->id();

      return error_code::success;
    }
    catch ( const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unknown error";
    }
  }


  int vw_model::request_slates_decision(const char *event_id, uint32_t slot_count, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status)
  {
    try {
      pooled_vw vw(_vw_pool, _vw_pool.get_or_create());

      // Get a ranked list of action_ids and corresponding pdf
      vw->rank_decisions(event_id, slot_count, features, actions_ids, action_pdfs);

      model_version = vw->id();

      return error_code::success;
    }
    catch ( const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unknown error";
    }
  }


}}
