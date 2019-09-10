
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

# Devirtualization is an optimization that changes the vtable if the compiler decides a function
# doesn't need to be virtual. This is incompatible with the mocking framework used in testing as it
# makes the vtable structure unpredictable
if(MSVC)
  # MSVC doesn't optimize aggresively enough for this bug to be an issue right now.
else()
  target_compile_options(rltest-onnx PRIVATE -fno-devirtualize)
endif()

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

add_test(rltest-onnx rltest-onnx WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})