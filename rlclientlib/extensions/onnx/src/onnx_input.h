#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include <core/session/onnxruntime_cxx_api.h>

#include "api_status.h"

namespace reinforcement_learning { namespace onnx {
  using byte_t = unsigned char;
  using bytes_t = std::vector<unsigned char>;
  using tensor_data_t = std::pair<bytes_t, bytes_t>;

  /**
   * Check whether the provided bytes contain enough data to represent an
   * array of elements of type element_t
   */
  template <typename element_t>
  inline bool check_array_size(const bytes_t& bytes, size_t& element_count)
  {
    element_count = bytes.size() / sizeof(element_t);
    
    // The number of bytes in the dimensions array does not fit evenly into an 
    // array of elements of type element_t
    return element_count * sizeof(element_t) == bytes.size();
  }

  /**
   * Check whether the provided bytes contains exactly expected_element_count
   * elements of type element_t
   */
  template <typename element_t>
  inline bool check_array_size(const bytes_t& bytes, size_t expected_element_count, size_t& element_count)
  {
    check_array_size<element_t>(bytes, element_count);

    return (element_count == expected_element_count);
  }

  // TODO: Support reading type information for the tensor (and later map/sequence)
  using value_t = float;

  class onnx_input_builder
  {
  public:
    onnx_input_builder(const Ort::MemoryInfo& allocator_info) : _memory_info{allocator_info}
    {}

  public:
    std::vector<const char*> input_names() const;
    std::vector<Ort::Value> inputs() const;

    inline size_t input_count() const
    {
      return count();
    }

    inline size_t count() const
    {
      return _inputs.size();
    }

  public:
    inline void push_input(const std::string& input_name, const tensor_data_t& input)
    {
      _input_names.push_back(input_name);
      _inputs.push_back(input);
    }

  private:
    std::vector<std::string> _input_names{};
    std::vector<tensor_data_t> _inputs{};

    const Ort::MemoryInfo& _memory_info;
  };

  int read_tensor_notation(const char* tensor_notation, onnx_input_builder& input_context, api_status* status = nullptr);
}}