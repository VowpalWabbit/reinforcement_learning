/**
 * @brief RL Inference API definition.
 *
 * @file slates_loop.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once
#include "action_flags.h"
#include "base_loop.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "future_compat.h"
#include "multi_slot_response.h"
#include "multi_slot_response_detailed.h"
#include "sender.h"

#include <functional>
#include <memory>

namespace reinforcement_learning
{
class slates_loop : public base_loop
{
public:
  using base_loop::base_loop;

  /**
   * @brief Choose an action from the given set for each slot, given a list of actions, slots,
   * action features, slot features and context features. The inference library chooses an action
   * per slot by sampling the probability distribution produced per slot. The corresponding event_id should be used when
   * reporting the outcome for each slot.
   * @param event_id  The unique identifier for this interaction.  The same event_id should be used when
   *                  reporting the outcome for this action.
   * @param context_json Contains slots, slot_features, slot ids, actions, action features and context features in json
   * format
   * @param flags Action flags (see action_flags.h)
   * @param resp Decision response contains the chosen action per slot, probability distribution used for sampling
   * actions and ranked actions.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int request_multi_slot_decision(loop_str event_id, loop_str context_json, unsigned int flags,
      multi_slot_response& resp, api_status* status = nullptr);
  int request_multi_slot_decision(
      loop_str event_id, loop_str context_json, multi_slot_response& resp, api_status* status = nullptr);
  int request_multi_slot_decision(
      loop_str context_json, unsigned int flags, multi_slot_response& resp, api_status* status = nullptr);
  int request_multi_slot_decision(loop_str context_json, multi_slot_response& resp, api_status* status = nullptr);

  int request_multi_slot_decision(loop_str event_id, loop_str context_json, unsigned int flags,
      multi_slot_response& resp, const int* baseline_actions, size_t baseline_actions_size,
      api_status* status = nullptr);

  int request_multi_slot_decision(loop_str event_id, loop_str context_json, unsigned int flags,
      multi_slot_response_detailed& resp, api_status* status = nullptr);
  int request_multi_slot_decision(
      loop_str event_id, loop_str context_json, multi_slot_response_detailed& resp, api_status* status = nullptr);
  int request_multi_slot_decision(
      loop_str context_json, unsigned int flags, multi_slot_response_detailed& resp, api_status* status = nullptr);
  int request_multi_slot_decision(
      loop_str context_json, multi_slot_response_detailed& resp, api_status* status = nullptr);

  int request_multi_slot_decision(loop_str event_id, loop_str context_json, unsigned int flags,
      multi_slot_response_detailed& resp, const int* baseline_actions, size_t baseline_actions_size,
      api_status* status = nullptr);

  /**
   * @brief Report the outcome for the top action.
   *
   * @param event_id  The unique event_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param outcome Outcome serialized as a string
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(loop_str event_id, loop_str outcome, api_status* status = nullptr);

  /**
   * @brief Report the outcome for the top action.
   *
   * @param event_id  The unique event_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param outcome Outcome as float
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(loop_str event_id, float outcome, api_status* status = nullptr);

private:
  const std::vector<int> default_baseline_vector = std::vector<int>();
  static std::vector<int> c_array_to_vector(
      const int* c_array, size_t array_size);  //! Convert baseline_actions from c array to std vector.
};
}  // namespace reinforcement_learning
