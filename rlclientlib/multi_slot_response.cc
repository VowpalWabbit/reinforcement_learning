#include "multi_slot_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning
{
  slot_entry::slot_entry(const std::string& id, uint32_t action_id, float probability)
    : _id(id)
    , _action_id(action_id)
    , _probability(probability) {
  }

  const char* slot_entry::get_id() const {
    return _id.c_str();
  }

  uint32_t slot_entry::get_action_id() const {
    return _action_id;
  }

  void slot_entry::set_action_id(uint32_t action_id) {
    _action_id = action_id;
  }

  void slot_entry::set_probability(float prob) {
    _probability = prob;
  }

  float slot_entry::get_probability() const {
    return _probability;
  }

  void multi_slot_response::set_event_id(const char* event_id) {
    _event_id = event_id;
  }

  void multi_slot_response::set_event_id(std::string&& event_id) {
    _event_id = event_id;
  }

  const char* multi_slot_response::get_event_id() const {
    return _event_id.c_str();
  }

  void multi_slot_response::set_model_id(const char* model_id) {
    _model_id = model_id;
  }

  void multi_slot_response::set_model_id(std::string&& model_id) {
    _model_id = model_id;
  }

  const char* multi_slot_response::get_model_id() const {
    return _model_id.c_str();
  }

  void multi_slot_response::clear() {
    _decision.clear();
    _model_id.clear();
    _event_id.clear();
  }

  void multi_slot_response::push_back(const std::string& id, uint32_t action_id, float prob) {
    _decision.emplace_back(id, action_id, prob);
  }

  size_t multi_slot_response::size() const {
    return _decision.size();
  }

  multi_slot_response::const_iterator_t multi_slot_response::begin() const {
    return { _decision };
  }

  multi_slot_response::iterator_t multi_slot_response::begin() {
    return { _decision };
  }

  multi_slot_response::const_iterator_t multi_slot_response::end() const {
    return { _decision, _decision.size() };
  }

  multi_slot_response::iterator_t multi_slot_response::end() {
    return { _decision, _decision.size() };
  }
}
