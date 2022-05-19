#pragma once

#include <cstddef>
#include <iterator>
#include <vector>

#include "ranking_response.h"
#include "vw/common/vwvis.h"

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
    VW_DLL_PUBLIC void push_back(const char* event_id, uint32_t action_id, float prob);

    VW_DLL_PUBLIC size_t size() const;

    VW_DLL_PUBLIC void set_model_id(const char* model_id);
    VW_DLL_PUBLIC void set_model_id(std::string&& model_id);
    VW_DLL_PUBLIC const char* get_model_id() const;

    VW_DLL_PUBLIC void clear();
    VW_DLL_PUBLIC const_iterator_t begin() const;
    VW_DLL_PUBLIC iterator_t begin();
    VW_DLL_PUBLIC const_iterator_t end() const;
    VW_DLL_PUBLIC iterator_t end();

    VW_DLL_PUBLIC decision_response(decision_response&&) noexcept;
    VW_DLL_PUBLIC decision_response& operator=(decision_response&&) noexcept;
    decision_response(const decision_response&) = delete;
    decision_response& operator =(const decision_response&) = delete;
  };
}
