#include "slates_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning
{
  slates_slot_response::slates_slot_response(uint32_t _slot_id, uint32_t _action_id, float _probability)
    : slot_id(_slot_id)
    , action_id(_action_id)
    , probability(_probability) {
  }

  uint32_t slates_slot_response::get_slot_id() const {
    return slot_id;
  }

  uint32_t slates_slot_response::get_action_id() const {
    return action_id;
  }

  float slates_slot_response::get_probability() const {
    return probability;
  }

  void slates_response::set_event_id(const char* event_id) {
    _event_id = event_id;
  }

  void slates_response::set_event_id(std::string&& event_id) {
    _event_id = event_id;
  }

  const char* slates_response::get_event_id() const {
    return _event_id.c_str();
  }

  void slates_response::set_model_id(const char* model_id) {
    _model_id = model_id;
  }

  void slates_response::set_model_id(std::string&& model_id) {
    _model_id = model_id;
  }

  const char* slates_response::get_model_id() const {
    return _model_id.c_str();
  }

  void slates_response::clear() {
    _decision.clear();
    _model_id.clear();
    _event_id.clear();
  }

  void slates_response::push_back(uint32_t action_id, float prob) {
    _decision.emplace_back(_decision.size(), action_id, prob);
  }

  size_t slates_response::size() const {
    return _decision.size();
  }

  slates_response::const_iterator_t slates_response::begin() const {
    return { _decision };
  }

  slates_response::iterator_t slates_response::begin() {
    return { _decision };
  }

  slates_response::const_iterator_t slates_response::end() const {
    return { _decision, _decision.size() };
  }

  slates_response::iterator_t slates_response::end() {
    return { _decision, _decision.size() };
  }
} // namespace reinforcement_learning
