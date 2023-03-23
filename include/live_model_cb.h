/**
 * @brief RL Inference API definition.
 *
 * @file live_model_cb.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once
#include "action_flags.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "future_compat.h"
#include "live_model_base.h"
#include "ranking_response.h"
#include "rl_string_view.h"
#include "sender.h"

#include <functional>
#include <memory>

namespace reinforcement_learning
{
class live_model_cb : public live_model_base
{
public:
  using live_model_base::live_model_base;

  /**
   * @brief Choose an action, given a list of actions, action features and context features. The
   * inference library chooses an action by creating a probability distribution over the actions
   * and then sampling from it.
   * @param event_id  The unique identifier for this interaction.  The same event_id should be used when
   *                  reporting the outcome for this action.
   * @param context_json Contains action, action features and context features in json format
   * @param resp Ranking response contains the chosen action, probability distribution used for sampling actions and
   * ranked actions
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int choose_rank(const char* event_id, string_view context_json, ranking_response& resp, api_status* status = nullptr);

  /**
   * @brief Choose an action, given a list of actions, action features and context features. The
   * inference library chooses an action by creating a probability distribution over the actions
   * and then sampling from it.  A unique event_id will be generated and returned in the ranking_response.
   * The same event_id should be used when reporting the outcome for this action.
   *
   * @param context_json Contains action, action features and context features in json format
   * @param resp Ranking response contains the chosen action, probability distribution used for sampling actions and
   * ranked actions
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int choose_rank(
      string_view context_json, ranking_response& resp, api_status* status = nullptr);  // event_id is auto-generated

  /**
   * @brief Choose an action, given a list of actions, action features and context features. The
   * inference library chooses an action by creating a probability distribution over the actions
   * and then sampling from it.
   * @param event_id  The unique identifier for this interaction.  The same event_id should be used when
   *                  reporting the outcome for this action.
   * @param context_json Contains action, action features and context features in json format
   * @param flags Action flags (see action_flags.h)
   * @param resp Ranking response contains the chosen action, probability distribution used for sampling actions and
   * ranked actions
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int choose_rank(const char* event_id, string_view context_json, unsigned int flags, ranking_response& resp,
      api_status* status = nullptr);

  /**
   * @brief Choose an action, given a list of actions, action features and context features. The
   * inference library chooses an action by creating a probability distribution over the actions
   * and then sampling from it.  A unique event_id will be generated and returned in the ranking_response.
   * The same event_id should be used when reporting the outcome for this action.
   * @param context_json Contains action, action features and context features in json format
   * @param flags Action flags (see action_flags.h)
   * @param resp Ranking response contains the chosen action, probability distribution used for sampling actions and
   * ranked actions
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int choose_rank(string_view context_json, unsigned int flags, ranking_response& resp,
      api_status* status = nullptr);  // event_id is auto-generated

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
