#pragma once

#include "container_iterator.h"
#include "slot_ranking.h"
#include <cstddef>
#include <iterator>
#include <vector>
#include <string>

namespace reinforcement_learning {
	class api_status;

	class multi_slot_response_detailed {
	private:
		using coll_t = std::vector<slot_ranking>;

		std::string _event_id;
		std::string _model_id;
		coll_t _decision;

	public:
		using iterator_t = container_iterator<slot_ranking, coll_t>;
		using const_iterator_t = const_container_iterator<slot_ranking, coll_t>;

		multi_slot_response_detailed() = default;
		~multi_slot_response_detailed() = default;


		void resize(size_t new_size);

		size_t size() const;

		void set_event_id(const char* event_id);
		void set_event_id(std::string&& event_id);
		const char* get_event_id() const;

		void set_model_id(const char* model_id);
		void set_model_id(std::string&& model_id);
		const char* get_model_id() const;

		int set_slot_at_index(const unsigned int index, slot_ranking&& slot, api_status* status = nullptr);

		void clear();
		const_iterator_t begin() const;
		iterator_t begin();
		const_iterator_t end() const;
		iterator_t end();

		multi_slot_response_detailed(multi_slot_response_detailed&&) noexcept;
		multi_slot_response_detailed& operator=(multi_slot_response_detailed&&) noexcept;

		/**
		* @brief Copy constructor is removed since implementation will be deleted twice
		*/
		multi_slot_response_detailed(const multi_slot_response_detailed&) = delete;

		/**
		* @brief assignment operator is removed since implementation will be deleted twice
		*/
		multi_slot_response_detailed& operator =(const multi_slot_response_detailed&) = delete;


	};

}
