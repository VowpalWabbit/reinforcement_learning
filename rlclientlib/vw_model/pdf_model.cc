#include "pdf_model.h"
#include "err_constants.h"
#include "object_factory.h"
#include "sampling.h"
#include "ranking_response.h"
#include "trace_logger.h"
#include "str_util.h"

namespace reinforcement_learning { namespace model_management {

  pdf_model::pdf_model(i_trace* trace_logger) :
    _trace_logger(trace_logger), _vw(new safe_vw("--json --quiet --cb_adf")) {
  }

  int pdf_model::update(const model_data& data, api_status* status) {
    return error_code::success;
  }

  int pdf_model::choose_rank(uint64_t rnd_seed, const char* features, ranking_response& response, api_status* status) {
    try {
      // TODO: is there a simpler way to get a vw object if we do not actually need to deal with model mgmt?
      //pooled_vw vw(_vw_pool, _vw_pool.get_or_create());

      // Rank actions using the model.  Should generate a pdf
      std::vector<int> action_ids;
      std::vector<float> pdf;

      // Get a ranked list of action_ids and corresponding pdf
      _vw->parse_context_with_pdf(features, action_ids, pdf);

      ::reinforcement_learning::sample_and_populate_response(rnd_seed, action_ids, pdf, response, _trace_logger, status);
      
      // TODO Should we provide some mechanism for the user to inject the model ID?
      // response.set_model_id(vw->id());

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
