#pragma once

#include <cstddef>
#include <iterator>
#include <vector>

#include "ranking_response.h"

namespace reinforcement_learning {
  class api_status;

  struct slot_response {
  public:
    ~slot_response() = default;

    slot_response(string_view _slot_id, uint32_t _action_id, float _probability);

    string_view get_slot_id() const;
    uint32_t get_action_id() const;
    float get_probability() const;
  private:
    //! slot_id
    const std::string slot_id;
    //! action id
    uint32_t action_id;
    //! probability associated with the action id
    float probability;
  };

  class decision_response {
  private:
    using coll_t = std::vector<slot_response>;

    std::string _model_id;
    coll_t _decision;

  public:
    using iterator_t = container_iterator<slot_response, coll_t>;
    using const_iterator_t = const_container_iterator<slot_response, coll_t>;

    decision_response() = default;
    ~decision_response() = default;

    // Cannot copy ranking_response, so must do a move here.
    void push_back(string_view event_id, uint32_t action_id, float prob);

    size_t size() const;

    void set_model_id(string_view model_id);
    void set_model_id(std::string&& model_id);
    string_view get_model_id() const;

    void clear();
    const_iterator_t begin() const;
    iterator_t begin();
    const_iterator_t end() const;
    iterator_t end();

    decision_response(decision_response&&) noexcept;
    decision_response& operator=(decision_response&&) noexcept;
    decision_response(const decision_response&) = delete;
    decision_response& operator =(const decision_response&) = delete;
  };
}
