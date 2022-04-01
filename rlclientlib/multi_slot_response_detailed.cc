#include "multi_slot_response_detailed.h"
#include "slot_ranking.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning
{
  using coll_t = std::vector<slot_ranking>;

  void multi_slot_response_detailed::set_event_id(string_view event_id) {
    _event_id = std::string(event_id);
  }

  void multi_slot_response_detailed::set_event_id(std::string&& event_id) {
    _event_id = event_id;
  }

  string_view multi_slot_response_detailed::get_event_id() const {
    return _event_id;
  }

  void multi_slot_response_detailed::set_model_id(string_view model_id) {
    _model_id = std::string(model_id);
  }

  void multi_slot_response_detailed::set_model_id(std::string&& model_id) {
    _model_id = model_id;
  }

  string_view multi_slot_response_detailed::get_model_id() const {
    return _model_id;
  }

  int multi_slot_response_detailed::set_slot_at_index(const unsigned int index, slot_ranking&& slot, api_status* status) {
    if (index >= _decision.size()) {
      RETURN_ERROR_ARG(nullptr, status, slot_index_out_of_bounds_error, "Slot index out of bounds");
    }
    _decision[index] = std::move(slot);
    return error_code::success;
  }

  void multi_slot_response_detailed::clear() {
    _model_id.clear();
    _event_id.clear();
    _decision.clear();
  }

  size_t multi_slot_response_detailed::size() const {
    return _decision.size();
  }

  multi_slot_response_detailed::const_iterator_t multi_slot_response_detailed::begin() const {
    return { _decision };
  }

  multi_slot_response_detailed::iterator_t multi_slot_response_detailed::begin() {
    return { _decision };
  }

  multi_slot_response_detailed::const_iterator_t multi_slot_response_detailed::end() const {
    return { _decision, _decision.size() };
  }

  multi_slot_response_detailed::iterator_t multi_slot_response_detailed::end() {
    return { _decision, _decision.size() };
  }

  void multi_slot_response_detailed::resize(size_t new_size) {
    _decision.resize(new_size);
  }
}
