#pragma once
#include <cstddef>
#include <stdint.h>

#include <utility>
#include <vector>
#include <string>

// Declare const pointer for internal linkage
namespace reinforcement_learning {
  class ranking_response;
  class api_status;
}

namespace reinforcement_learning { namespace model_management {
    class model_data {
      public:
        // Get data
        char* data() const;
        size_t data_sz() const;
        uint32_t refresh_count() const;

        void data_sz(size_t fillsz);
        void increment_refresh_count();

        // Allocate
        char* alloc(size_t desired);
        void free();

        model_data();
        ~model_data();

        model_data(model_data const& other);
        model_data& operator=(model_data const& other);

        model_data(model_data&& other) noexcept
          : _data(other._data),
            _data_sz(other._data_sz),
            _refresh_count(other._refresh_count) {}

        model_data& operator=(model_data&& other) noexcept {
          if (this != &other) {
            std::swap(_data, other._data);
            std::swap(_data_sz, other._data_sz);
            std::swap(_refresh_count, other._refresh_count);
          }

          return *this;
        }

      private:
        char * _data = nullptr;
        size_t _data_sz = 0;
        uint32_t _refresh_count = 0;
    };

    //! The i_data_transport interface provides the way to retrieve the data for a model from some source.
    class i_data_transport {
    public:
      virtual int get_data(model_data& data, api_status* status = nullptr) = 0;
      virtual ~i_data_transport() = default;
    };

    enum class model_type_t {
      UNKNOWN,
      CB,
      CCB,
      SLATES,
      CA
    };

    //! The i_model interfaces provides the resolution from the raw model_data to a consumable object.
    class i_model {
    public:
      virtual int update(const model_data& data, bool& model_ready, api_status* status = nullptr) = 0;
      virtual int choose_rank(uint64_t rnd_seed, const char* features, std::vector<int>& action_ids, std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) = 0;
      virtual int choose_continuous_action(const char* features, float& action, float& pdf_value, std::string& model_version, api_status* status = nullptr) = 0;
      virtual int request_decision(const std::vector<const char*>& event_ids, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) = 0;
      virtual int request_multi_slot_decision(const char* event_id, uint32_t slot_count, const char* features, std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) = 0;
      virtual model_type_t model_type() const = 0;
      virtual ~i_model() = default;
    };
}}
