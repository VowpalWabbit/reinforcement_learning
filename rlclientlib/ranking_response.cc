#include "ranking_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning {
  
  ranking_response::ranking_response(char const* event_id)
    : _slot_impl { slot_ranking(event_id) } {}

  char const* ranking_response::get_event_id() const {
  return _slot_impl.get_id();
  }

  void ranking_response::set_event_id(char const* event_id) {
    _slot_impl.set_id(event_id);
  }

  int ranking_response::get_chosen_action_id(size_t& action_id, api_status* status) const {
    return _slot_impl.get_chosen_action_id(action_id, status);
  }
  
  int ranking_response::set_chosen_action_id(size_t action_id, api_status* status) {
    return _slot_impl.set_chosen_action_id(action_id, status);
  }

  int ranking_response::set_chosen_action_id_unchecked(size_t action_id, api_status* status) {
    return _slot_impl.set_chosen_action_id(action_id, status);
  }

  void ranking_response::push_back(const size_t action_id, const float prob) {
    _slot_impl.push_back(action_id, prob);
  }

  size_t ranking_response::size() const {
    return _slot_impl.size();
  }

  void ranking_response::set_model_id(const char* model_id) {
    _model_id = model_id;
  }

  void ranking_response::set_model_id(std::string&& model_id) {
    _model_id.assign(std::move(model_id));
  }

  const char* ranking_response::get_model_id() const {
    return _model_id.c_str();
  }

  void ranking_response::clear() {
    _slot_impl.clear();
    _model_id.clear();
  }

  ranking_response::ranking_response(ranking_response&& tmp) noexcept :
    _slot_impl(std::move(tmp._slot_impl)),
    _model_id(std::move(tmp._model_id)){}

  ranking_response& ranking_response::operator=(ranking_response&& tmp) noexcept {
    std::swap(_slot_impl, tmp._slot_impl);
    std::swap(_model_id, tmp._model_id);
    return *this;
  }

  using iterator = ranking_response::iterator;
  using const_iterator = ranking_response::const_iterator;

  const_iterator ranking_response::begin() const {
    return _slot_impl.begin();
  }

  iterator ranking_response::begin() {
    return _slot_impl.begin();
  }

  const_iterator ranking_response::end() const {
    return _slot_impl.end();
  }

  iterator ranking_response::end() {
    return _slot_impl.end();
  }
}
