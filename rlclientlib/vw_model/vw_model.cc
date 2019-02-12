#include "vw_model.h"
#include "err_constants.h"
#include "object_factory.h"
#include "sampling.h"
#include "ranking_response.h"
#include "trace_logger.h"
#include "str_util.h"

namespace reinforcement_learning { namespace model_management {

  vw_model::vw_model(i_trace* trace_logger) :
    _vw_pool(nullptr) , _trace_logger(trace_logger) {
  }

  int vw_model::update(const model_data& data, api_status* status) {
    try {
      TRACE_INFO(_trace_logger, utility::concat("Recieved new model data. With size ", data.data_sz()));
      
      // safe_vw_factory will create a copy of the model data to use for vw object construction.
      _vw_pool.update_factory(new safe_vw_factory(std::move(data)));
    }
    catch(const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, model_update_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(_trace_logger, status, model_update_error) << "Unkown error";
    }

    return error_code::success;
  }

  int vw_model::choose_rank(uint64_t rnd_seed, const char* features, ranking_response& response, api_status* status) {
    try {
      pooled_vw vw(_vw_pool, _vw_pool.get_or_create());

      // Rank actions using the model.  Should generate a pdf
      std::vector<int> action_ids;
      std::vector<float> pdf;

      // Get a ranked list of action_ids and corresponding pdf
      vw->rank(features, action_ids, pdf);

      // TODO: Should there be an i_pdf_model? It seems like the ability to integrate at the PDF level easily would be nice.
      ::reinforcement_learning::sample_and_populate_response(rnd_seed, action_ids, pdf, response, _trace_logger, status);
      
      response.set_model_id(vw->id());

      return error_code::success;
    }
    catch ( const std::exception& e) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unkown error";
    }
  }
}}
