#pragma once
#include <boost/test/unit_test.hpp>

#include "api_status.h"
#include "err_constants.h"
#include "global_fixture.h"
#include "onnx_extension.h"
#include "onnx_input.h"

#include <cpprest/asyncrt_utils.h>
#include <onnxruntime_cxx_api.h>

#include <map>
#include <sstream>
#include <tuple>
#include <vector>

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#endif

namespace r = reinforcement_learning;
namespace o = reinforcement_learning::onnx;

using dimensions = std::vector<int64_t>;
using tensor_raw = std::vector<float>;

template <typename string_t>
using expectation_t = std::tuple<string_t, dimensions, tensor_raw>;

template <typename string_t>
using expectations = std::vector<expectation_t<string_t>>;

static const std::vector<const char*> TestEmptyNames{};
static const std::vector<Ort::Value> TestEmptyTensors{};

template <typename T>
size_t byte_size(const std::vector<T>& v)
{
  return (sizeof(T) * v.size());
}

inline void require_status(const r::api_status& status, int expected_code)
{
  BOOST_REQUIRE_MESSAGE(status.get_error_code() == expected_code, status.get_error_msg());
}

inline void require_success(const r::api_status& status) { require_status(status, r::error_code::success); }

template <typename string_t>
inline void validate_input_context(
    o::onnx_input_builder& input_context, size_t expected_count, std::vector<string_t> expected_names)
{
  size_t input_count = input_context.input_count();
  BOOST_REQUIRE_EQUAL(input_count, expected_count);

  std::vector<const char*> input_names = input_context.input_names();
  BOOST_REQUIRE_EQUAL(input_names.size(), input_count);
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
      input_names.cbegin(), input_names.cend(), expected_names.cbegin(), expected_names.cend());
}

inline void validate_empty(o::onnx_input_builder& input_context,
    const Ort::MemoryInfo& memory_info = GlobalConfig::instance()->get_memory_info())
{
  validate_input_context(input_context, 0, TestEmptyNames);
  std::vector<Ort::Value> values;

  r::api_status status;
  input_context.allocate_inputs(values, memory_info, &status);
  require_success(status);

  BOOST_REQUIRE_EQUAL_COLLECTIONS(values.cbegin(), values.cend(), TestEmptyTensors.cbegin(), TestEmptyTensors.cend());
}

inline void validate_tensor(Ort::Value& parsed_value, dimensions expected_dimensions, tensor_raw expected_values)
{
  Ort::TensorTypeAndShapeInfo type_shape_info = parsed_value.GetTensorTypeAndShapeInfo();
  size_t parsed_element_count = type_shape_info.GetElementCount();

  BOOST_REQUIRE_EQUAL(parsed_element_count, expected_values.size());

  std::vector<int64_t> parsed_dimensions = type_shape_info.GetShape();
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
      parsed_dimensions.cbegin(), parsed_dimensions.cend(), expected_dimensions.cbegin(), expected_dimensions.cend());

  float* parsed_values = parsed_value.GetTensorMutableData<float>();
  BOOST_REQUIRE_EQUAL_COLLECTIONS(
      parsed_values, parsed_values + parsed_element_count, expected_values.cbegin(), expected_values.cend());
}

template <typename string_t>
inline void validate_tensors(o::onnx_input_builder& input_context, expectations<string_t> expectations,
    const Ort::MemoryInfo& memory_info = GlobalConfig::instance()->get_memory_info())
{
  std::map<string_t, size_t> input_name_map;
  size_t expected_input_count = input_context.input_count();
  std::vector<const char*> input_names = input_context.input_names();

  for (size_t i = 0; i < expected_input_count; i++)
  {
    input_name_map.insert(std::make_pair(string_t{input_names[i]}, i));
  }

  std::vector<Ort::Value> inputs;

  r::api_status status;
  input_context.allocate_inputs(inputs, memory_info, &status);
  require_success(status);

  // If these are not equal, it means we are not able to allocate the tensors.
  BOOST_REQUIRE_EQUAL(inputs.size(), expected_input_count);

  size_t expectation_count = expectations.size();
  for (int i = 0; i < expectation_count; i++)
  {
    string_t name;
    dimensions expected_dimensions;
    tensor_raw expected_values;

    std::tie(name, expected_dimensions, expected_values) = expectations[i];

    auto position = input_name_map.find(name);
    BOOST_REQUIRE_MESSAGE(position != input_name_map.end(), "Input to be checked was not present in set of inputs");
    size_t input_position = position->second;

    validate_tensor(inputs[input_position], expected_dimensions, expected_values);
  }
}

#ifdef WIN32
inline std::wstring s2ws(const std::string& s)
{
  int len;
  int slength = static_cast<int>(s.length());
  len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
  std::wstring r(len, L'\0');
  MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
  return r;
}

inline std::string ws2s(const std::wstring& s)
{
  int len;
  int slength = static_cast<int>(s.length());
  len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
  std::string r(len, '\0');
  WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
  return r;
}
#endif

inline std::string to_base64(const o::bytes_t& bytes)
{
#ifdef WIN32
  return ws2s(::utility::conversions::to_base64(bytes));
#else
  return ::utility::conversions::to_base64(bytes);
#endif
}
inline o::bytes_t from_base64(const std::string& str)
{
#ifdef WIN32
  return ::utility::conversions::from_base64(s2ws(str));
#else
  return ::utility::conversions::from_base64(str);
#endif
}

template <bool roundtrip = false>
std::string encode_tensor_data(const dimensions& dimensions, const tensor_raw& values)
{
  unsigned char* dimensions_buffer = (unsigned char*)dimensions.data();
  o::bytes_t dimensions_bytes(dimensions_buffer, dimensions_buffer + byte_size(dimensions));
  std::string dimensions_base64 = to_base64(dimensions_bytes);

  unsigned char* values_buffer = (unsigned char*)values.data();
  o::bytes_t values_bytes(values_buffer, values_buffer + byte_size(values));
  std::string values_base64 = to_base64(values_bytes);

  if (roundtrip)
  {
    o::bytes_t dimensions_bytes_back = from_base64(dimensions_base64);
    o::bytes_t values_bytes_back = from_base64(values_base64);

    BOOST_REQUIRE_EQUAL_COLLECTIONS(
        values_bytes.cbegin(), values_bytes.cend(), values_bytes_back.cbegin(), values_bytes_back.cend());

    float* values_data_back = (float*)values_bytes_back.data();
    size_t values_count = values.size();

    BOOST_REQUIRE_EQUAL_COLLECTIONS(values_data_back, values_data_back + values_count, values.cbegin(), values.cend());

    Ort::Value tensor = Ort::Value::CreateTensor<float>(GlobalConfig::instance()->get_memory_info(), values_data_back,
        values_count, dimensions.data(), dimensions.size());
    float* tensor_data = tensor.GetTensorMutableData<float>();

    BOOST_REQUIRE_EQUAL_COLLECTIONS(tensor_data, tensor_data + values_count, values.cbegin(), values.cend());
  }

  std::stringstream result_stream;
  result_stream << '\"' << dimensions_base64 << ';' << values_base64 << '\"';

  return result_stream.str();
}

template <typename string_t, bool roundtrip = false>
std::string create_tensor_notation(const expectations<string_t>& expectations)
{
  std::stringstream tensor_notation_builder;
  tensor_notation_builder << '{';

  size_t inputs_count = expectations.size();
  for (int i = 0; i < inputs_count; i++)
  {
    string_t name;
    dimensions dimensions;
    tensor_raw rawdata;

    std::tie(name, dimensions, rawdata) = expectations[i];

    tensor_notation_builder << '\"' << name << "\":" << encode_tensor_data<roundtrip>(dimensions, rawdata);
  }

  tensor_notation_builder << '}';

  return tensor_notation_builder.str();
}
