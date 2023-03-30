#include "slates_loop.h"

#include "live_model_impl.h"

namespace reinforcement_learning
{
int slates_loop::request_multi_slot_decision(
    loop_str event_id, loop_str context_json, unsigned int flags, multi_slot_response& resp, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_multi_slot_decision(
      event_id.str, string_view(context_json.str, context_json.size), flags, resp, slates_loop::default_baseline_vector, status);
}

int slates_loop::request_multi_slot_decision(
    loop_str event_id, loop_str context_json, multi_slot_response& resp, api_status* status)
{
  return request_multi_slot_decision(event_id, context_json, action_flags::DEFAULT, resp, status);
}

int slates_loop::request_multi_slot_decision(
    loop_str context_json, unsigned int flags, multi_slot_response& resp, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_multi_slot_decision(string_view(context_json.str, context_json.size), flags, resp, slates_loop::default_baseline_vector, status);
}

int slates_loop::request_multi_slot_decision(loop_str context_json, multi_slot_response& resp, api_status* status)
{
  return request_multi_slot_decision(context_json, action_flags::DEFAULT, resp, status);
}

int slates_loop::request_multi_slot_decision(loop_str event_id, loop_str context_json, unsigned int flags,
    multi_slot_response& resp, const int* baseline_actions, size_t baseline_actions_size, api_status* status)
{
  INIT_CHECK();
  std::vector<int> baseline_vector = c_array_to_vector(baseline_actions, baseline_actions_size);
  if (event_id.str == nullptr)
  {
    return _pimpl->request_multi_slot_decision(string_view(context_json.str, context_json.size), flags, resp, baseline_vector, status);
  }
  return _pimpl->request_multi_slot_decision(event_id.str, string_view(context_json.str, context_json.size), flags, resp, baseline_vector, status);
}

int slates_loop::request_multi_slot_decision(loop_str event_id, loop_str context_json, unsigned int flags,
    multi_slot_response_detailed& resp, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_multi_slot_decision(
      event_id.str, string_view(context_json.str, context_json.size), flags, resp, slates_loop::default_baseline_vector, status);
}

int slates_loop::request_multi_slot_decision(
    loop_str event_id, loop_str context_json, multi_slot_response_detailed& resp, api_status* status)
{
  return request_multi_slot_decision(event_id, context_json, action_flags::DEFAULT, resp, status);
}

int slates_loop::request_multi_slot_decision(
    loop_str context_json, unsigned int flags, multi_slot_response_detailed& resp, api_status* status)
{
  INIT_CHECK();
  return _pimpl->request_multi_slot_decision(string_view(context_json.str, context_json.size), flags, resp, slates_loop::default_baseline_vector, status);
}

int slates_loop::request_multi_slot_decision(
    loop_str context_json, multi_slot_response_detailed& resp, api_status* status)
{
  return request_multi_slot_decision(context_json, action_flags::DEFAULT, resp, status);
}

int slates_loop::request_multi_slot_decision(loop_str event_id, loop_str context_json, unsigned int flags,
    multi_slot_response_detailed& resp, const int* baseline_actions, size_t baseline_actions_size, api_status* status)
{
  INIT_CHECK();
  std::vector<int> baseline_vector = c_array_to_vector(baseline_actions, baseline_actions_size);
  if (event_id.str == nullptr)
  {
    return _pimpl->request_multi_slot_decision(string_view(context_json.str, context_json.size), flags, resp, baseline_vector, status);
  }
  return _pimpl->request_multi_slot_decision(event_id.str, string_view(context_json.str, context_json.size), flags, resp, baseline_vector, status);
}

int slates_loop::report_outcome(loop_str event_id, loop_str outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id.str, outcome.str, status);
}

int slates_loop::report_outcome(loop_str event_id, float outcome, api_status* status)
{
  INIT_CHECK();
  return _pimpl->report_outcome(event_id.str, outcome, status);
}

std::vector<int> slates_loop::c_array_to_vector(const int* c_array, size_t array_size)
{
  if (c_array == nullptr) { return {}; }
  return std::vector<int>(c_array, c_array + array_size);
}
}  // namespace reinforcement_learning
