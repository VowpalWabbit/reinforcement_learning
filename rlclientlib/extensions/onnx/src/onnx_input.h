#pragma once
#include <algorithm>
#include <string>
#include <vector>

#include <core/session/onnxruntime_cxx_api.h>

#include "api_status.h"

namespace reinforcement_learning { namespace onnx {
  using byte_t = unsigned char;
  using bytes_t = std::vector<byte_t>;
  using tensor_data_t = std::pair<bytes_t, bytes_t>;

  /**
   * Check whether the provided bytes map exactly to a whole number of elements
   * of type element_t.
   */
  template <typename element_t>
  inline bool check_array_packing(const bytes_t& bytes, size_t& element_count)
  {
    element_count = (bytes.size() * sizeof(byte_t)) / sizeof(element_t);
    
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
    return check_array_packing<element_t>(bytes, element_count) 
           && (element_count == expected_element_count);
  }

  // TODO: Support reading type information for the tensor (and later map/sequence)
  using value_t = float;

  class onnx_input_builder
  {
  public:
    onnx_input_builder(i_trace* trace_logger) : _trace_logger{trace_logger}
    {}

  public:
    std::vector<const char*> input_names() const;
    int allocate_inputs(std::vector<Ort::Value>& result, const Ort::MemoryInfo& allocator_info, api_status* status = nullptr) const;

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

    i_trace* _trace_logger;
  };

  int read_tensor_notation(const char* tensor_notation, onnx_input_builder& input_context, api_status* status = nullptr);
}}