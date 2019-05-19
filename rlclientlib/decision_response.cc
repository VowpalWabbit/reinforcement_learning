#include "decision_response.h"
#include "api_status.h"
#include "err_constants.h"

namespace reinforcement_learning {
  void decision_response::push_back(const size_t action_id, const float prob) {

  }

  size_t decision_response::size() const {
    return 1;
  }

  void decision_response::set_model_id(const char* model_id)
  {

  }
  void decision_response::set_model_id(std::string&& model_id) {

  }
  const char* decision_response::get_model_id() const {
    return nullptr;
  }

  void decision_response::clear() {
    return _decision.clear();
  }

  decision_response::const_iterator_t decision_response::begin() const {
    return _decision.cbegin();
  }

  decision_response::iterator_t decision_response::begin() {
    return _decision.begin();
  }

  decision_response::const_iterator_t decision_response::end() const {
    return _decision.cend();
  }

  decision_response::iterator_t decision_response::end() {
    return _decision.end();
  }

  decision_response::decision_response(decision_response&&) noexcept {

  }

  decision_response& decision_response::operator=(decision_response&&) noexcept {
    return *this;
  }
}
