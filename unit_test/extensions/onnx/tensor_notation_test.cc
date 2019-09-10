#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "test_helpers.h"

#include "tensor_notation.h"

BOOST_AUTO_TEST_CASE(null_pointer) {
  // Arrange
  Ort::MemoryInfo memory_info{nullptr};
  o::OnnxRtInputContext ic{memory_info};

  // Act
  r::api_status status;
  o::read_tensor_notation(nullptr, &ic, &status);
  
  // Assert
  require_success(status);
  validate_empty(ic);
}

BOOST_AUTO_TEST_CASE(empty_string) {
  // Arrange
  Ort::MemoryInfo memory_info{nullptr};
  o::OnnxRtInputContext ic{memory_info};

  // Act
  r::api_status status;
  o::read_tensor_notation("", &ic, &status);
  
  // Assert
  require_success(status);
  validate_empty(ic);
}

BOOST_AUTO_TEST_CASE(empty_object) {
  // Arrange
  Ort::MemoryInfo memory_info{nullptr};
  o::OnnxRtInputContext ic{memory_info};

  // Act
  r::api_status status;
  o::read_tensor_notation("{}", &ic, &status);
  
  // Assert
  require_success(status);
  validate_empty(ic);
}

const auto SimpleVectorNotation = R"(
  {
    'abc':'BAAAAAAAAAA=;AACAP2ZmBkBmZoZAmpkRwQ=='
  }
  )";

BOOST_AUTO_TEST_CASE(simple_vector) 
{
  // Arrange
  std::vector<int64_t> expected_dimensions({4});
  std::vector<float> expected_values({1.0f, 2.1f, 4.2f, -9.1f});

  o::OnnxRtInputContext ic{TestMemoryInfo};

  // Act
  r::api_status status;
  o::read_tensor_notation(SimpleVectorNotation, &ic, &status);
  
  // Assert
  require_success(status);
  validate_input_context(ic, 1, std::vector<std::string>({"abc"}));

  std::vector<Ort::Value> parsed_inputs = ic.inputs();
  BOOST_REQUIRE_EQUAL(parsed_inputs.size(), 1);
}

BOOST_AUTO_TEST_CASE(roundtrip_tensor_data)
{
  // The goal of this test is to ensure that the assumptions we make about roundtripping the underlying 
  // types in the tensor_notation parser hold.
  std::vector<int64_t> dimensions({4});
  std::vector<float> values({1.0f, 2.1f, 4.2f, -9.1f});

  encode_tensor_data<true>(dimensions, values);
}

template <typename string_t>
inline void run_tensor_notation_test(Ort::MemoryInfo& memory_info, expectations<string_t> expectations)
{
  // Arrange
  std::string tensor_notation = create_tensor_notation(expectations);
  std::vector<string_t> input_names;

  std::for_each(expectations.cbegin(), expectations.cend(), 
    [&input_names](expectation_t<string_t> expectation)
    {
      input_names.push_back(std::get<0>(expectation));
    });

  o::OnnxRtInputContext ic{memory_info};

  // Act
  r::api_status status;
  o::read_tensor_notation(tensor_notation.c_str(), &ic, &status);

  // Assert
  require_success(status);
  validate_input_context(ic, 1, input_names);
  validate_tensors(ic, expectations);
}

BOOST_AUTO_TEST_CASE(higher_rank_tensor)
{
  // Arrange
  const int size = 28*28;
  const std::string input_name = "Input3";

  dimensions dims{1, 1, 28, 28};
  tensor_raw rawdata{};

  rawdata.reserve(size);
  for (int i = 0; i < size; i++)
  {
    rawdata.push_back((float)i / (float)size);
  };

  expectations<std::string> expectations{ std::make_tuple(input_name, dims, rawdata) };

  run_tensor_notation_test(TestMemoryInfo, expectations);
}