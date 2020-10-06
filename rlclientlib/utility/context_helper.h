#pragma once
#include "api_status.h"

#include <vector>
#include <map>
#include <utility>

namespace reinforcement_learning {
  class i_trace;
  namespace utility {

    //! This struct collects all sort of relevant data we need about a context json.
    struct ContextInfo {
      //! Each pair is the start and end offsets of a JSON object. IE the offsets to '{' and '}'
      typedef std::vector<std::pair<size_t, size_t>> index_vector_t;

      //! The index to each element in the _multi array
      index_vector_t actions;
      //! The index to each element in the _slots array
      index_vector_t slots;
  };

  int get_action_count(size_t& count, const char *context, i_trace* trace, api_status* status = nullptr);
  int get_event_ids(const char* context, std::map<size_t, std::string>& event_ids, i_trace* trace, api_status* status);
  int get_slot_count(size_t& count, const char *context, i_trace* trace, api_status* status = nullptr);
  int validate_multi_before_slots(const char *context, i_trace* trace, api_status* status = nullptr);
  int get_context_info(const char *context, ContextInfo &info, i_trace* trace = nullptr, api_status* status = nullptr);
}}
