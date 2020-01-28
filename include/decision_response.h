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

    slot_response(const char* _slot_id);

    const char* get_slot_id() const;
    uint32_t get_action_id() const;
    float get_probability() const;

    void push_back(uint32_t action_id, float prob);

  private:
    //! slot_id
    const std::string slot_id;
    using coll_t = std::vector<action_prob>;
    coll_t _ranking;

  public:
    using iterator = container_iterator<action_prob, coll_t>;
    using const_iterator = const_container_iterator<action_prob, coll_t>;
    //! Returns an iterator pointing to the first element of the (action, probability) collection
    const_iterator begin() const;
    iterator begin();

    //! Returns an iterator referring to the past-the-end element of the (action, probability) collection.
    const_iterator end() const;
    iterator end();

    size_t size() const;
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

    slot_response& push_back(const char* event_id);

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
  };
}
