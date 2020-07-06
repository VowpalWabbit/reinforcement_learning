#pragma once

#include <cstddef>
#include <iterator>
#include <vector>

#include "ranking_response.h"

namespace reinforcement_learning {
  class api_status;

  struct slates_slot_response {
  public:
    ~slates_slot_response() = default;

    slates_slot_response(uint32_t _slot_id, uint32_t _action_id, float _probability);

    uint32_t get_slot_id() const;
    uint32_t get_action_id() const;
    float get_probability() const;
  private:
    //! slot_id
    uint32_t slot_id;
    //! action id
    uint32_t action_id;
    //! probability associated with the action id
    float probability;
  };

  class slates_response {
  private:
    using coll_t = std::vector<slates_slot_response>;

    std::string _event_id;
    std::string _model_id;
    coll_t _decision;

  public:
    using iterator_t = container_iterator<slates_slot_response, coll_t>;
    using const_iterator_t = const_container_iterator<slates_slot_response, coll_t>;

    slates_response() = default;
    ~slates_response() = default;

    // push_back calls must be done in slot order
    void push_back(uint32_t action_id, float prob);

    size_t size() const;

    void set_event_id(const char* event_id);
    void set_event_id(std::string&& event_id);
    const char* get_event_id() const;

    void set_model_id(const char* model_id);
    void set_model_id(std::string&& model_id);
    const char* get_model_id() const;

    void clear();
    const_iterator_t begin() const;
    iterator_t begin();
    const_iterator_t end() const;
    iterator_t end();

    slates_response(slates_response&&) noexcept;
    slates_response& operator=(slates_response&&) noexcept;
    slates_response(const slates_response&) = default;
    slates_response& operator =(const slates_response&) = default;
  };
}
