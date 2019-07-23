#pragma once

#include <cstddef>
#include <iterator>
#include <vector>

#include "ranking_response.h"

namespace reinforcement_learning {
  class api_status;

  class decision_response {
  public:
    using coll_t = std::vector<ranking_response>;
    using iterator_t = coll_t::iterator;
    using const_iterator_t = coll_t::const_iterator;

    decision_response() = default;
    ~decision_response() = default;

    // Cannot copy ranking_response, so must do a move here.
    void push_back(ranking_response&& r_response);

    size_t size() const;

    void set_model_id(const char* model_id);
    void set_model_id(std::string&& model_id);
    const char* get_model_id() const;

    void clear();
    const_iterator_t begin() const;
    iterator_t begin();
    const_iterator_t end() const;
    iterator_t end();

    decision_response(decision_response&&) noexcept;
    decision_response& operator=(decision_response&&) noexcept;
    decision_response(const decision_response&) = delete;
    decision_response& operator =(const decision_response&) = delete;
  private:
    std::string _model_id;
    coll_t _decision;
  };
}
