/**
 * @brief RL Inference API definition.
 *
 * @file live_model_episodic.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once
#include "action_flags.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "future_compat.h"
#include "live_model_base.h"
#include "multistep.h"
#include "rl_string_view.h"
#include "sender.h"

#include <functional>
#include <memory>

namespace reinforcement_learning
{
class live_model_episodic : public live_model_base
{
public:
  using live_model_base::live_model_base;

  // multistep
  int request_episodic_decision(const char* event_id, const char* previous_id, string_view context_json,
      ranking_response& resp, episode_state& episode, api_status* status = nullptr);
  int request_episodic_decision(const char* event_id, const char* previous_id, string_view context_json,
      unsigned int flags, ranking_response& resp, episode_state& episode, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on a pair of primary and secondary indentifiers.
   * This identifier pair is problem specific.
   * For CCB, the primary is the event id and the secondary is the index of the slot.
   *
   * @param primary_id  The unique primary_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param secondary_id Index of the partial outcome.
   * @param outcome Outcome as float.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(const char* primary_id, int secondary_id, float outcome, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on a pair of primary and secondary indentifiers.
   * This identifier pair is problem specific.
   * For CCB, the primary is the event id and the secondary is the index of the slot.
   *
   * @param primary_id  The unique primary_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param secondary_id Index of the partial outcome.
   * @param outcome Outcome as float.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(const char* primary_id, const char* secondary_id, float outcome, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on a pair of primary and secondary indentifiers.
   * This identifier pair is problem specific.
   * For CCB, the primary is the event id and the secondary is the index of the slot.
   *
   * @param primary_id  The unique primary_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param secondary_id Index of the partial outcome.
   * @param outcome Outcome as float.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(const char* primary_id, int secondary_id, const char* outcome, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on a pair of primary and secondary indentifiers.
   * This identifier pair is problem specific.
   * For CCB, the primary is the event id and the secondary is the index of the slot.
   *
   * @param primary_id  The unique primary_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param secondary_id Index of the partial outcome.
   * @param outcome Outcome as float.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(
      const char* primary_id, const char* secondary_id, const char* outcome, api_status* status = nullptr);
};

}  // namespace reinforcement_learning
