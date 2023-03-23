/**
 * @brief RL Inference API definition.
 *
 * @file live_model_ca.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once
#include "action_flags.h"
#include "continuous_action_response.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "future_compat.h"
#include "live_model_base.h"
#include "multi_slot_response.h"
#include "multi_slot_response_detailed.h"
#include "rl_string_view.h"
#include "sender.h"

#include <functional>
#include <memory>

namespace reinforcement_learning
{
class live_model_ca : public live_model_base
{
public:
  using live_model_base::live_model_base;
  
  /**
   * @brief (DEPRECATED) Choose an action from a continuous range, given a list of context features
   * The inference library chooses an action by sampling the probability density function produced per continuous action
   * range. The corresponding event_id should be used when reporting the outcome for the continuous action.
   * @param event_id  The unique identifier for this interaction.  The same event_id should be used when
   *                  reporting the outcome for this action.
   * @param context_json Contains context features in json format
   * @param flags Action flags (see action_flags.h)
   * @param response Continuous action response contains the chosen action and the probability density value of the
   * chosen action location from the continuous range.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  RL_DEPRECATED("New unified example builder interface is coming")
  int request_continuous_action(const char* event_id, string_view context_json, unsigned int flags,
      continuous_action_response& response, api_status* status = nullptr);

  /**
   * @brief (DEPRECATED) Choose an action from a continuous range, given a list of context features
   * The inference library chooses an action by sampling the probability density function produced per continuous action
   * range. The corresponding event_id should be used when reporting the outcome for the continuous action.
   * @param event_id  The unique identifier for this interaction.  The same event_id should be used when
   *                  reporting the outcome for this action.
   * @param context_json Contains context features in json format
   * @param response Continuous action response contains the chosen action and the probability density value of the
   * chosen action location from the continuous range.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  RL_DEPRECATED("New unified example builder interface is coming")
  int request_continuous_action(const char* event_id, string_view context_json, continuous_action_response& response,
      api_status* status = nullptr);

  /**
   * @brief (DEPRECATED) Choose an action from a continuous range, given a list of context features
   * The inference library chooses an action by sampling the probability density function produced per continuous action
   * range. A unique event_id will be generated and returned in the continuous_action_response. The same event_id should
   * be used when reporting the outcome for this action.
   * @param context_json Contains context features in json format
   * @param flags Action flags (see action_flags.h)
   * @param response Continuous action response contains the chosen action and the probability density value of the
   * chosen action location from the continuous range.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  RL_DEPRECATED("New unified example builder interface is coming")
  int request_continuous_action(
      string_view context_json, unsigned int flags, continuous_action_response& response, api_status* status = nullptr);

  /**
   * @brief (DEPRECATED) Choose an action from a continuous range, given a list of context features
   * The inference library chooses an action by sampling the probability density function produced per continuous action
   * range. A unique event_id will be generated and returned in the continuous_action_response. The same event_id should
   * be used when reporting the outcome for this action.
   * @param context_json Contains context features in json format
   * @param response Continuous action response contains the chosen action and the probability density value of the
   * chosen action location from the continuous range.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  RL_DEPRECATED("New unified example builder interface is coming")
  int request_continuous_action(
      string_view context_json, continuous_action_response& response, api_status* status = nullptr);

  /**
   * @brief Report that action was taken.
   *
   * @param event_id  The unique event_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_action_taken(const char* event_id, api_status* status = nullptr);

  /**
   * @brief Report the outcome for the top action.
   *
   * @param event_id  The unique event_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param outcome Outcome serialized as a string
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(const char* event_id, const char* outcome, api_status* status = nullptr);

  /**
   * @brief Report the outcome for the top action.
   *
   * @param event_id  The unique event_id used when choosing an action should be presented here.  This is so that
   *                  the action taken can be matched with feedback received.
   * @param outcome Outcome as float
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(const char* event_id, float outcome, api_status* status = nullptr);
};
}  // namespace reinforcement_learning
