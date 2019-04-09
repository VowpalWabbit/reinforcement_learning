#include "vw_model.h"
#include "err_constants.h"
#include "object_factory.h"
#include "ranking_response.h"
#include "trace_logger.h"
#include "str_util.h"

namespace reinforcement_learning { namespace model_management {

  vw_model::vw_model(i_trace* trace_logger) :
    _vw_pool(nullptr), _trace_logger(trace_logger) {
  }

  int vw_model::update(const model_data& data, bool& model_ready, api_status* status) {
    try {
      TRACE_INFO(_trace_logger, utility::concat("Received new model data. With size ", data.data_sz()));

      if (data.data_sz() > 0)
      {
        // safe_vw_factory will create a copy of the model data to use for vw object construction.
        _vw_pool.update_factory(new safe_vw_factory(std::move(data)));
        model_ready = true;
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

  int vw_model::choose_decisions(const char* features, std::vector<std::vector<int>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status)
  {
    try {
      pooled_vw vw(_vw_pool, _vw_pool.get_or_create());

      // Get a ranked list of action_ids and corresponding pdf
      vw->rank_decisions(features, actions_ids, action_pdfs);

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
