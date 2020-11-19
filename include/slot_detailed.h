/**
 * @brief slot_detailed definition. slot_detailed contains the cosen action, along with the action probability distribution for a slot. 
 */
#pragma once
#include "container_iterator.h"
#include <cstddef>
#include <iterator>
#include <vector>
#include <string>

namespace reinforcement_learning {
	class api_status;

	/**
	 * @brief Holds (action, probability) pairs, POD used for extern "C"
	 */
	struct action_prob_d {
		//! action id
		size_t action_id;
		//! probability associated with the action id
		float probability;
	};

	/**
	 * @brief Holds (action, probability) pairs.
	 */
	struct action_prob : public action_prob_d {
		inline action_prob(size_t action_id, float probability) {
			this->action_id = action_id;
			this->probability = probability;
		};
	};

	/**
	 * @brief
	 * slot_detailed contains all the actions and distribution from with the action was sampled.  It also contains the chosen action id and
	 * the unique join_id representing the choice.
	 */
	class slot_detailed {
	public:
		slot_detailed() = default;
		~slot_detailed() = default;
		/**
			* @brief Construct a new slot detailed object.
			*
			* @param join_id The unique identifier for this interaction.  This event_id must be used when reporting the outcome for this action
			*/
		slot_detailed(char const* join_id);

		/**
		* @brief Unique join_id for this slot detailed instance.
		* This join_id must be used when calling report_outcome so it can be joined with the chosen action
		* @return const char*
		*/
		const char* get_join_id() const;

		/**
		* @brief Set the join_id.  (This is set internally by the API)
		* @param event_id
		*/
		void set_join_id(const char* join_id);

		/**
		* @brief Get the chosen action id.
		*
		* @param action_id Chosen action id
		* @param status Optional field with detailed string description if there is an error
		* @return int Error code
		*/
		int get_chosen_action_id(size_t& action_id, api_status* status = nullptr) const; // id of the top action chosen by the model

		/**
		* @brief Set the chosen action id.  (This is set internally by the API)
		*
		* @param action_id Chosen action id
		* @param status Optional field with detailed string description if there is an error
		* @return int Error code
		*/
		int set_chosen_action_id(size_t action_id, api_status* status = nullptr); // id of the top action chosen by the model

		/**
		* @brief Set the chosen action id, but do not verify the index fits within the ranking.  (This is set internally by the API)
		* This is used in CCB where subsequent ranking_responses have subsets of the orignal actionset.
		*
		* @param action_id Chosen action id
		* @param status Optional field with detailed string description if there is an error
		* @return int Error code
		*/
		int set_chosen_action_id_unchecked(size_t action_id, api_status* = nullptr);

		/**
		* @brief Add (action id, probability) pair to the slot (This is set internally by the API)
		*
		* @param action_id
		* @param prob
		*/
		void push_back(const size_t action_id, const float prob);

		/**
		* @brief Size of the action collection.
		*
		* @return size_t
		*/
		size_t size() const;

		/**
		* @brief Clear the slot detailed object so that it can be reused.
		* The goal is to reuse response without reallocating as much as possible.
		*/
		void clear();

		/**
		* @brief Move construct a new slot detailed object.
		* The underlying data is taken from the rvalue reference.
		*/
		slot_detailed(slot_detailed&&) noexcept;

		/**
		* @brief Move assignment operator for slot detailed.
		* The underlying data is taken from rvalue reference, and then it is cleared.
		* @return ranking_response&
		*/
		slot_detailed& operator=(slot_detailed&&) noexcept;

		/**
		* @brief Copy constructor is removed since implementation will be deleted twice
		*/
		slot_detailed(const slot_detailed&) = delete;

		/**
		* @brief assignment operator is removed since implementation will be deleted twice
		*/
		slot_detailed& operator =(const slot_detailed&) = delete;

	private:
		std::string _join_id;
		size_t _chosen_action_id;
		using coll_t = std::vector<action_prob>;
		coll_t _ranking;


	public:
		using iterator = container_iterator<action_prob, coll_t>;
		using const_iterator = const_container_iterator<action_prob, coll_t>;
		//! Returns an iterator pointing to the first element of the (action, probability) collection
		const_iterator begin() const;
		iterator begin();

		//! Returns an iterator referring to the past-the-end element of the (action, probability) collection.
		const_iterator end() const;
		iterator end();
	};
}
