#pragma once

#include <cstddef>
#include <iterator>
#include <vector>
#include <string>
#include "vw/common/vwvis.h"

namespace reinforcement_learning {
  class api_status;

  /**
   * @brief request_continuous_action() returns the continuous action choice using a continuous_action_response object.
   * continuous_action_response contains the chosen continuous action and the pdf value of the density function at the chosen action's location.
   * continuous_action_response also contains the unique event_id representing the choice.
   * This unique event_id must be used to report back outcomes against this choice for the online trainer.
   */
  class continuous_action_response {
  public:
    continuous_action_response() = default;
    ~continuous_action_response() = default;

    /**
     * @brief Construct a new continuous action response object.
     *
     * @param event_id The unique identifier for this interaction. This event_id must be used when reporting the outcome for this action
     */
    VW_DLL_PUBLIC continuous_action_response(char const* event_id);

    /**
     * @brief Get the chosen continuous action
     *
     * @return float
     */
    VW_DLL_PUBLIC float get_chosen_action() const;

    /**
     * @brief Set the chosen continuous action. (This is set internally by the API)
     *
     * @param action Chosen continuous action
     */
    VW_DLL_PUBLIC void set_chosen_action(float action);

    /**
     * @brief Get the pdf value at the chosen continuous action location
     *
     * @return float
     */
    VW_DLL_PUBLIC float get_chosen_action_pdf_value() const;

    /**
     * @brief Set the chosen continuous action pdf value. (This is set internally by the API)
     *
     * @param pdf_value
     */
    VW_DLL_PUBLIC void set_chosen_action_pdf_value(float pdf_value);

    /**
     * @brief Set the event_id. (This is set internally by the API)
     * @param event_id
     */

    VW_DLL_PUBLIC void set_event_id(const char* event_id);
    /**
     * @brief Set the event_id. (This is set internally by the API)
     * Input event_id is left in an unspecified but valid state.
     * @param event_id
     */
    VW_DLL_PUBLIC void set_event_id(std::string&& event_id);

    /**
     * @brief Unique event_id for this continuous action request.
     * This event_id must be used when calling report_outcome so it can be joined with the chosen action
     * @return const char*
     */
    VW_DLL_PUBLIC const char* get_event_id() const;

    /**
     * @brief Set the model_id.
     * Every call to request an action is associated with a unique model used to predict. A unique model_id
     * is associated with each unique model. (This is set internally by the API)
     * @param model_id
     */
    VW_DLL_PUBLIC void set_model_id(const char* model_id);

    /**
     * @brief Set the model_id.
     * Every call to request an action is associated with a unique model used to predict. A unique model_id
     * is associated with each unique model. (This is set internally by the API)
     * Input model_id is left in an unspecified but valid state.
     * @param model_id
     */
    VW_DLL_PUBLIC void set_model_id(std::string&& model_id);

    /**
     * @brief Get the model_id.
     * Every call to request an action is associated with a unique model used to predict. A unique model_id
     * is associated with each unique model. (This is set internally by the API)
     * @return const char*
     */
    VW_DLL_PUBLIC const char* get_model_id() const;

    /**
     * @brief Clear the response object so that it can be reused.
     * The goal is to reuse response without reallocating as much as possible.
     */
    VW_DLL_PUBLIC void clear();

    /**
     * @brief Default move constructor for continuous action response object.
     */
    continuous_action_response(continuous_action_response&&) = default;

    /**
     * @brief Default move assignment operator for continuous action response object.
     * @return ranking_response&
     */
    continuous_action_response& operator=(continuous_action_response&&) = default;

    /**
     * @brief Copy constructor is removed to avoid objects being deleted twice
     */
    continuous_action_response(const continuous_action_response&) = delete;

    /**
     * @brief assignment operator is removed to avoid objects being deleted twice
     */
    continuous_action_response& operator =(const continuous_action_response&) = delete;
  private:
    float _chosen_action;
    float _chosen_action_pdf_value;
    std::string _model_id;
    std::string _event_id;
  };
}
