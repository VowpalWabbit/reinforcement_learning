#include "sampling.h"
#include "err_constants.h"
#include "ranking_response.h"
#include "trace_logger.h"
#include "api_status.h"
#include "explore.h"

namespace e = exploration;
namespace reinforcement_learning {

int populate_response(size_t chosen_action_index, std::vector<int>& action_ids, std::vector<float>& pdf, std::string&& model_id, ranking_response& response, i_trace* trace_logger, api_status* status) {
  for ( size_t idx = 0; idx < pdf.size(); ++idx ) {
    response.push_back(action_ids[idx], pdf[idx]);
  }

  RETURN_IF_FAIL(response.set_chosen_action_id(action_ids[chosen_action_index]));
  response.set_model_id(std::move(model_id));
  return error_code::success;
}

int sample_and_populate_response(uint64_t rnd_seed, std::vector<int>& action_ids, std::vector<float>& pdf, std::string&& model_id, ranking_response& response, i_trace* trace_logger, api_status* status) {
    try {
      // Pick a slot using the pdf. NOTE: sample_after_normalizing() can change the pdf
      uint32_t chosen_index;
      auto scode = e::sample_after_normalizing(rnd_seed, std::begin(pdf), std::end(pdf), chosen_index);

      if ( S_EXPLORATION_OK != scode ) {
        RETURN_ERROR_LS(trace_logger, status, exploration_error) << scode;
      }

      RETURN_IF_FAIL(populate_response(chosen_index, action_ids, pdf, std::move(model_id), response, trace_logger, status));

      // Swap values in first position with values in chosen index
      scode = e::swap_chosen(std::begin(response), std::end(response), chosen_index);

      if ( S_EXPLORATION_OK != scode ) {
        RETURN_ERROR_LS(trace_logger, status, exploration_error) << "Exploration error code: " << scode;
      }

      return error_code::success;
    }
    catch ( const std::exception& e) {
      RETURN_ERROR_LS(trace_logger, status, model_rank_error) << e.what();
    }
    catch ( ... ) {
      RETURN_ERROR_LS(trace_logger, status, model_rank_error) << "Unknown error";
    }
  }
}
