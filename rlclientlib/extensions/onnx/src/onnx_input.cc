#include "onnx_input.h"

#include <numeric>
#include <sstream>

#include "err_constants.h"
#include "tensor_parser.h"

namespace reinforcement_learning { namespace onnx {

std::vector<const char*> onnx_input_builder::input_names() const
{
  int count = 0;

  std::vector<const char*> result;
  result.reserve(input_count());

  std::for_each(_input_names.cbegin(), _input_names.cend(), 
    [&result, &count](const std::string& str)
    {
      result.push_back(str.c_str());
    });

  return result;
}

int onnx_input_builder::allocate_inputs(std::vector<Ort::Value>& result, const Ort::MemoryInfo& memory_info, api_status* status) const
{
  // TODO: enable non-float tensors (see tensor_notation.cc for hook location)
  int count = 0;
  result.reserve(input_count());

  bool failed = false;

  for (const tensor_data_t& tensor : _inputs)
  {
    const bytes_t& dimensions_bytes = tensor.first;
    const bytes_t& values_bytes = tensor.second;
        
    // Unpack the dimensions
    size_t rank;
    if (!check_array_packing<int64_t>(dimensions_bytes, rank))
    {
      RETURN_ERROR_LS(_trace_logger, status, extension_error) 
        << "Invalid tensor dimension data packing for input '" << _input_names[result.size()] 
        << "'. Expecting multiple of " << sizeof(int64_t) << ". Got " << dimensions_bytes.size() << ".";
    }

    // TODO: Should we validate that dimensions are all positive numbers during model load?
    int64_t* dimensions = (int64_t*)dimensions_bytes.data();
    size_t expected_values_count = rank == 0 ? 0 : std::accumulate(dimensions, dimensions + rank, 1, std::multiplies<size_t>());

    // Unpack the data
    size_t values_count = 0;
    if (!check_array_size<value_t>(values_bytes, expected_values_count, values_count))
    {
      RETURN_ERROR_LS(_trace_logger, status, extension_error) 
        << "Invalid tensor value packing/data for input '" << _input_names[result.size()] 
        << "'. Expecting multiple of " << sizeof(int64_t) << ". Got " << values_bytes.size()
        << ". Expecting " << expected_values_count << " elements. Got " << values_count << ".";
    }

    value_t* values = (value_t*)values_bytes.data();
    result.push_back(std::move(Ort::Value::CreateTensor<value_t>(memory_info, values, values_count, dimensions, rank)));
  }

  return error_code::success;
}

int read_tensor_notation(string_view tensor_notation, onnx_input_builder& input_context, api_status* status)
{
  if (tensor_notation == nullptr)
  {
    // Treat empty lines as empty examples, similar to VW
    return error_code::success;
  }

  tensor_parser::parser_context ctx(std::string(tensor_notation), input_context);
  if (!tensor_parser::parse(ctx))
  {
    std::string error_detail = "OnnxExtension: Failed to deserialize input";

    if (status != nullptr)
    {
      std::stringstream detail_stream;
      
      detail_stream << ": ";
      for (const auto& error_info : ctx.errors())
      {
        detail_stream << std::endl << error_info;
      }

      error_detail += detail_stream.str();
    }
    
    RETURN_ERROR_LS(nullptr, status, extension_error) << error_detail;
  }

  return error_code::success;
}

}}