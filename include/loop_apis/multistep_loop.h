/**
 * @brief RL Inference API definition.
 *
 * @file cb_loop.h
 * @author Peter Chang et al
 * @date 2023-05-02
 */
#pragma once
#include "action_flags.h"
#include "base_loop.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "future_compat.h"
#include "multistep.h"
#include "ranking_response.h"
#include "sender.h"

#include <functional>
#include <memory>

namespace reinforcement_learning
{
class multistep_loop : public base_loop
{
public:
  using base_loop::base_loop;

  /**
   * @brief Choose an action, given a list of actions, action features and context features. The
   * inference library chooses an action by creating a probability distribution over the actions
   * and then sampling from it.
   * @param event_id  The unique identifier for this interaction. The same event_id should be used when
   *                  reporting the outcome for this action.
   * @param previous_id The unique identifier for the previous interaction in the current episode.
   * @param context_json Contains action, action features and context features in json format
   * @param resp Ranking response contains the chosen action, probability distribution used for sampling actions and
   * ranked actions
   * @param episode_state Object containing the state information for the current episode. The same object must
   *                      be passed in for every interaction within a single episode
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int request_episodic_decision(str_view event_id, str_view previous_id, str_view context_json,
      ranking_response& resp, episode_state& episode, api_status* status = nullptr);

  /**
   * @brief Choose an action, given a list of actions, action features and context features. The
   * inference library chooses an action by creating a probability distribution over the actions
   * and then sampling from it.
   * @param event_id  The unique identifier for this interaction. The same event_id should be used when
   *                  reporting the outcome for this action.
   * @param previous_id The unique identifier for the previous interaction in the current episode.
   * @param context_json Contains action, action features and context features in json format
   * @param flags Action flags (see action_flags.h)
   * @param resp Ranking response contains the chosen action, probability distribution used for sampling actions and
   * ranked actions
   * @param episode_state Object containing the state information for the current episode. The same object must
   *                      be passed in for every interaction within a single episode
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int request_episodic_decision(str_view event_id, str_view previous_id, str_view context_json,
      unsigned int flags, ranking_response& resp, episode_state& episode, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on a pair of episode and event identifiers.
   *        The outcome is associated with an individual interaction
   *
   * @param episode_id  The unique episode_id used when choosing an action should be presented here.
   * @param event_id Index of the partial outcome; an individual interaction within an episode.
   * @param outcome Outcome as float.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(str_view episode_id, str_view event_id, float outcome, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on a pair of episode and event identifiers.
   *        The outcome is associated with an individual interaction
   *
   * @param episode_id  The unique episode_id used when choosing an action should be presented here.
   * @param event_id Index of the partial outcome; an individual interaction within an episode.
   * @param outcome Outcome as string.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(str_view episode_id, str_view event_id, str_view outcome, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on a pair of episode and event identifiers.
   *        The outcome is associated with an individual interaction
   *
   * @param event_id Index of the partial outcome; an individual interaction within an episode.
   * @param episode_state Object containing the state information for the current episode. This object is the same
   *                      object that is passed in for the associated interaction
   * @param outcome Outcome as float.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(str_view event_id, episode_state& episode, float outcome, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on a pair of episode and event identifiers.
   *        The outcome is associated with an individual interaction
   *
   * @param event_id Index of the partial outcome; an individual interaction within an episode.
   * @param episode_state Object containing the state information for the current episode. This object is the same
   *                      object that is passed in for the associated interaction
   * @param outcome Outcome as string.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(str_view event_id, episode_state& episode, str_view outcome, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on the episode_id. This outcome is associated with the entire episode.
   *
   * @param episode_id  The unique episode_id used when choosing an action should be presented here.
   * @param outcome Outcome as float.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(str_view episode_id, float outcome, api_status* status = nullptr);

  /**
   * @brief Report outcome of a decision based on the episode_id. This outcome is associated with the entire episode.
   *
   * @param episode_id  The unique episode_id used when choosing an action should be presented here.
   * @param outcome Outcome as string.
   * @param status  Optional field with detailed string description if there is an error
   * @return int Return error code.  This will also be returned in the api_status object
   */
  int report_outcome(str_view episode_id, str_view outcome, api_status* status = nullptr);
};
}  // namespace reinforcement_learning
