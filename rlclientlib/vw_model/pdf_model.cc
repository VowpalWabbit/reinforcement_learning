#include "pdf_model.h"
#include "err_constants.h"
#include "object_factory.h"
#include "ranking_response.h"
#include "trace_logger.h"
#include "str_util.h"

namespace reinforcement_learning { namespace model_management {

  // We construct a VW object here to use the example parser to parse joined dsjson-style examples
  // to extract the PDF.
  pdf_model::pdf_model(i_trace* trace_logger, const utility::configuration&) :
    _trace_logger(trace_logger), _vw(new safe_vw("--json --quiet --cb_adf"))
  { }

  int pdf_model::update(const model_data& data, bool& model_ready, api_status* status) {
    model_ready = true;
    return error_code::success;
  }

  int pdf_model::choose_rank(
    uint64_t rnd_seed,
    const char* features,
    std::vector<int>& action_ids,
    std::vector<float>& action_pdf,
    std::string& model_version,
    api_status* status) {
    try
    {
      // Get a ranked list of action_ids and corresponding pdf
      _vw->parse_context_with_pdf(features, action_ids, action_pdf);

      model_version = _model_version.c_str();

      return error_code::success;
    }
    catch ( const std::exception& e)
    {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << e.what();
    }
    catch ( ... )
    {
      RETURN_ERROR_LS(_trace_logger, status, model_rank_error) << "Unknown error";
    }
  }


  // Not supported.
  int pdf_model::request_decision(const std::vector<const char*>& event_ids, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string & model_version, api_status * status)
  {
    return error_code::not_supported;
  }

  int pdf_model::request_multi_slot_decision(const char *event_id, uint32_t slot_count, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status)
  {
    return error_code::not_supported;
  }

  model_type_t pdf_model::model_type()
  {
    return model_type_t::CB;
  }
}}
