#include "onnx_input.h"
#include "err_constants.h"
#include "api_status.h"
#include "tensor_parser.h"

#include <sstream>
#include <numeric>
#include <assert.h>

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

std::vector<Ort::Value> onnx_input_builder::inputs() const
{
  // TODO: enable non-float tensors (see tensor_notation.cc for hook location)
  int count = 0;
  std::vector<Ort::Value> result;
  result.reserve(input_count());

  bool succeeded = false;

  for (const tensor_data_t& tensor : _inputs)
  {
    const bytes_t& dimensions_bytes = tensor.first;
    const bytes_t& values_bytes = tensor.second;
        
    // Unpack the dimensions
    size_t rank = dimensions_bytes.size() / sizeof(int64_t);
    if (!check_array_size<int64_t>(dimensions_bytes, rank))
    {
      // TODO: error
      break;
    }

    // TODO: Should we validate that dimensions are all positive numbers during model load?
    int64_t* dimensions = (int64_t*)dimensions_bytes.data();
    size_t expected_values_count = rank == 0 ? 0 : std::accumulate(dimensions, dimensions + rank, 1, std::multiplies<size_t>());

    // Unpack the data
    size_t values_count = 0;
    if (!check_array_size<value_t>(values_bytes, expected_values_count, values_count))
    {
      // TODO: error
      break;
    }

    value_t* values = (value_t*)values_bytes.data();
    result.push_back(std::move(Ort::Value::CreateTensor<value_t>(this->_memory_info, values, values_count, dimensions, rank)));

    succeeded = true;
  }

  if (!succeeded)
  {
    return {};
  }

  return result;
}

int read_tensor_notation(const char* tensor_notation, onnx_input_builder& input_context, api_status* status)
{
  if (tensor_notation == nullptr)
  {
    // Treat empty lines as empty examples, similar to VW
    return error_code::success;
  }

  tensor_parser::parser_context ctx(std::string(tensor_notation), input_context);
  if (!tensor_parser::parse(ctx))
  {
    std::string error_detail = "OnnxExtensions: Failed to deserialize input";

    if (status != nullptr)
    {
      std::stringstream detail_stream;
      
      detail_stream << ": ";
      for (const auto& error_detail : ctx.errors())
      {
        detail_stream << std::endl << error_detail;
      }

      error_detail += detail_stream.str();
    }
    
    RETURN_ERROR_LS(nullptr, status, extension_error) << error_detail;
  }

  return error_code::success;
}

}}