set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OnnxRuntime REQUIRED)

# If compiling on windows add the stdafx file
add_executable(rltest-onnx
  main.cc
  tensor_notation_test.cc
  mnist_inference_test.cc
  mock_helpers.cc
)

# Test Resources

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mnist_data/)

set(ONNX_EXTENSION_TEST_RESOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/mnist_data/mnist_model.onnx
)

add_custom_command(
  TARGET rltest-onnx POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${ONNX_EXTENSION_TEST_RESOURCE_FILES}
          ${CMAKE_CURRENT_BINARY_DIR}/mnist_data/
)

# Add the include directories from rlclientlib target for testing
target_include_directories(rltest-onnx
  PRIVATE
    $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:rlclientlib-onnx,INCLUDE_DIRECTORIES>
)

target_link_libraries(rltest-onnx
  PRIVATE
    rlclientlib-onnx
    #rlclientlib
    fakeit
    Boost::unit_test_framework
    Boost::system
)

add_custom_command(TARGET rltest-onnx POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  $<TARGET_FILE:onnxruntime>
  $<TARGET_FILE_DIR:rltest-onnx>
)

add_test(rltest-onnx rltest-onnx WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})