#pragma once
#include "multistep.h"

#include <cstdint>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

// Declare const pointer for internal linkage
namespace reinforcement_learning
{
class ranking_response;
class api_status;
}  // namespace reinforcement_learning

namespace reinforcement_learning
{
namespace model_management
{
class model_data
{
public:
  // Get data
  char* data();
  const char* data() const;
  size_t data_sz() const;
  uint32_t refresh_count() const;

  void data_sz(size_t fillsz);
  void increment_refresh_count();

  // Allocate
  char* alloc(size_t desired);
  void free();

  model_data() = default;
  ~model_data() = default;

  model_data(const model_data& other) = default;
  model_data(model_data&& other) noexcept = default;
  model_data& operator=(const model_data& other) noexcept = default;
  model_data& operator=(model_data&& other) noexcept = default;

private:
  std::vector<char> _data;
  uint32_t _refresh_count = 0;
};

//! The i_data_transport interface provides the way to retrieve the data for a model from some source.
class i_data_transport
{
public:
  virtual int get_data(model_data& data, api_status* status = nullptr) = 0;
  virtual ~i_data_transport() = default;
};

enum class model_type_t
{
  UNKNOWN,
  CB,
  CCB,
  SLATES,
  CA
};

enum class model_source
{
  AZURE,
  HTTP_API
};
//! The i_model interfaces provides the resolution from the raw model_data to a consumable object.
class i_model
{
public:
  virtual int update(const model_data& data, bool& model_ready, api_status* status = nullptr) = 0;
  virtual int choose_rank(const char* event_id, uint64_t rnd_seed, string_view features, std::vector<int>& action_ids,
      std::vector<float>& action_pdf, std::string& model_version, api_status* status = nullptr) = 0;
  virtual int choose_continuous_action(string_view features, float& action, float& pdf_value,
      std::string& model_version, api_status* status = nullptr) = 0;
  virtual int request_decision(const std::vector<const char*>& event_ids, string_view features,
      std::vector<std::vector<uint32_t>>& actions_ids, std::vector<std::vector<float>>& action_pdfs,
      std::string& model_version, api_status* status = nullptr) = 0;
  virtual int request_multi_slot_decision(const char* event_id, const std::vector<std::string>& slot_ids,
      string_view features, std::vector<std::vector<uint32_t>>& actions_ids,
      std::vector<std::vector<float>>& action_pdfs, std::string& model_version, api_status* status = nullptr) = 0;
  virtual int choose_rank_multistep(const char* event_id, uint64_t rnd_seed, string_view features,
      const episode_history& history, std::vector<int>& action_ids, std::vector<float>& action_pdf,
      std::string& model_version, api_status* status = nullptr) = 0;
  virtual model_type_t model_type() const = 0;
  virtual ~i_model() = default;
};
}  // namespace model_management
}  // namespace reinforcement_learning
