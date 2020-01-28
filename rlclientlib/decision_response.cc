#include "decision_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning
{
  slot_response::slot_response(const char* _slot_id)
    : slot_id(_slot_id) {
  }

  const char* slot_response::get_slot_id() const {
    return slot_id.c_str();
  }

  uint32_t slot_response::get_action_id() const {
    return _ranking.front().action_id;
  }

  float slot_response::get_probability() const {
    return _ranking.front().probability;
  }

  using iterator = slot_response::iterator;
  using const_iterator = slot_response::const_iterator;

  const_iterator slot_response::begin() const {
    return { _ranking };
  }

  iterator slot_response::begin() {
    return { _ranking };
  }

  const_iterator slot_response::end() const {
    return { _ranking, _ranking.size() };
  }

  iterator slot_response::end() {
    return { _ranking, _ranking.size() };
  }

  size_t slot_response::size() const {
    return _ranking.size();
  }

  void slot_response::push_back(uint32_t action_id, float prob) {
    _ranking.emplace_back(action_id, prob);
  }

  slot_response& decision_response::push_back(const char* event_id) {
    _decision.emplace_back(event_id);
    return _decision.back();
  }

  size_t decision_response::size() const {
    return _decision.size();
  }

  void decision_response::set_model_id(const char* model_id) {
    _model_id = model_id;
  }
  void decision_response::set_model_id(std::string&& model_id) {
    _model_id.assign(std::move(model_id));
  }
  const char* decision_response::get_model_id() const {
    return _model_id.c_str();
  }

  void decision_response::clear() {
    _decision.clear();
    _model_id.clear();
  }

  decision_response::const_iterator_t decision_response::begin() const {
    return { _decision };
  }

  decision_response::iterator_t decision_response::begin() {
    return { _decision };
  }

  decision_response::const_iterator_t decision_response::end() const {
    return { _decision, _decision.size() };
  }

  decision_response::iterator_t decision_response::end() {
    return { _decision, _decision.size() };
  }

  decision_response::decision_response(decision_response&& other) noexcept :
    _decision(std::move(other._decision)),
    _model_id(std::move(other._model_id))
  {}

  decision_response &decision_response::operator=(decision_response&& other) noexcept {
    std::swap(_model_id, other._model_id);
    std::swap(_decision, other._decision);
    return *this;
  }
} // namespace reinforcement_learning
