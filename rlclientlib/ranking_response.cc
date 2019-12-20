#include "ranking_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning {

  ranking_response::ranking_response(char const* event_id)
    : _event_id { event_id }, _chosen_action_id { 0 } {}

  char const* ranking_response::get_event_id() const {
    return _event_id.c_str();
  }

  int ranking_response::get_chosen_action_id(size_t& action_id, api_status* status) const {
    if ( _ranking.empty() ) {
      RETURN_ERROR_LS(nullptr, status, action_not_found);
    }
    action_id = _chosen_action_id;
    return error_code::success;
  }

  int ranking_response::set_chosen_action_id(size_t action_id, api_status* status) {
    if ( action_id >= _ranking.size() ) {
      RETURN_ERROR_LS(nullptr, status, action_out_of_bounds) << " id:" << action_id << ", size:" << _ranking.size();
    }

    _chosen_action_id = action_id;
    return error_code::success;
  }

  int ranking_response::set_chosen_action_id_unchecked(size_t action_id, api_status*) {
    _chosen_action_id = action_id;
    return error_code::success;
  }

  void ranking_response::set_event_id(char const* event_id) {
    _event_id = event_id;
  }

  void ranking_response::push_back(const size_t action_id, const float prob) {
    _ranking.emplace_back(action_id, prob);
  }

  size_t ranking_response::size() const {
    return _ranking.size();
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
    _event_id.clear();
    _chosen_action_id = 0;
    _model_id.clear();
    _ranking.clear();
  }

  ranking_response::ranking_response(ranking_response&& tmp) noexcept :
  _event_id(std::move(tmp._event_id)),
    _chosen_action_id(tmp._chosen_action_id),
    _model_id(std::move(tmp._model_id)),
    _ranking(std::move(tmp._ranking)) {}

  ranking_response& ranking_response::operator=(ranking_response&& tmp) noexcept {
    std::swap(_event_id, tmp._event_id);
    std::swap(_chosen_action_id, tmp._chosen_action_id);
    std::swap(_model_id, tmp._model_id);
    std::swap(_ranking, tmp._ranking);
    return *this;
  }

  using iterator = ranking_response::iterator;
  using const_iterator = ranking_response::const_iterator;

  const_iterator ranking_response::begin() const {
    return { _ranking };
  }

  iterator ranking_response::begin() {
    return { _ranking };
  }

  const_iterator ranking_response::end() const {
    return { _ranking, _ranking.size() };
  }

  iterator ranking_response::end() {
    return { _ranking, _ranking.size() };
  }
}
