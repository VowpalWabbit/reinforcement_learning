#include "continuous_action_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning {

  continuous_action_response::continuous_action_response(char const* event_id)
    : _event_id(event_id), _chosen_action(0), _chosen_action_pdf_value(0) {}

  float continuous_action_response::get_chosen_action() const
  {
    return _chosen_action;
  }

  void continuous_action_response::set_chosen_action(float action)
  {
    _chosen_action = action;
  }

  float continuous_action_response::get_chosen_action_pdf_value() const
  {
    return _chosen_action_pdf_value;
  }

  void continuous_action_response::set_chosen_action_pdf_value(float pdf_value)
  {
    _chosen_action_pdf_value = pdf_value;
  }

  void continuous_action_response::set_event_id(string_view event_id) {
    _event_id = std::string(event_id);
  }

  void continuous_action_response::set_event_id(std::string&& event_id) {
    _event_id = event_id;
  }

  const char* continuous_action_response::get_event_id() const {
    return _event_id.c_str();
  }

  void continuous_action_response::set_model_id(string_view model_id) {
    _model_id = std::string(model_id);
  }

  void continuous_action_response::set_model_id(std::string&& model_id) {
    _model_id = model_id;
  }

  const char* continuous_action_response::get_model_id() const {
    return _model_id.c_str();
  }

  void continuous_action_response::clear()
  {
    _event_id.clear();
    _model_id.clear();
    _chosen_action = 0.;
    _chosen_action_pdf_value = 0.;
  }
}