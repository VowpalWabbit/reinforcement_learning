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

    slot_response(const char* _slot_id, uint32_t _action_id, float _probability);

    const char* get_slot_id() const;
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
  public:
    using iterator_t = container_iterator<slot_response>;
    using const_iterator_t = const_container_iterator<slot_response>;

    decision_response() = default;
    ~decision_response() = default;

    // Cannot copy ranking_response, so must do a move here.
    void push_back(const char* event_id, uint32_t action_id, float prob);

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
    using coll_t = std::vector<slot_response>;

    std::string _model_id;
    coll_t _decision;
  };
}
