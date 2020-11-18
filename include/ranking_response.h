/**
 * @brief ranking_response definition. ranking_response is returned from choose_rank call.  It contains the chosen action and probability distribution.
 *
 * @file ranking_response.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once
#include "container_iterator.h"
#include "slot_detailed.h"
#include <cstddef>
#include <iterator>
#include <vector>
#include <string>

namespace reinforcement_learning {
  class api_status;

  /**
   * @brief choose_rank() returns the action choice using ranking_response.
   * ranking_response contains all the actions and distribution from with the action was sampled.  It also contains the chosen action id and
   * the unique event_id representing the choice.  This unique event_id must be used to report back outcomes against this choice for the online trainer.
   */
  class ranking_response : public slot_detailed {
  private:
	using slot_detailed::get_join_id;
	using slot_detailed::set_join_id;

  public:
	using slot_detailed::slot_detailed;
	~ranking_response() = default;
    /**
     * @brief Unique event_id for this ranking request.
     * This event_id must be used when calling report_outcome so it can be joined with the chosen action
     * @return const char*
     */
    const char* get_event_id() const;

    /**
     * @brief Set the event_id.  (This is set internally by the API)
     * @param event_id
     */
    void set_event_id(const char* event_id);
  };
}
