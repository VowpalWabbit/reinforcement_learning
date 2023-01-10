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

# Automatically set BOOST_TEST_DYN_LINK if the dependency is shared.
get_target_property(boost_test_target_type Boost::unit_test_framework TYPE)
if (boost_test_target_type STREQUAL SHARED_LIBRARY)
  message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
  target_compile_definitions(rltest-onnx PRIVATE BOOST_TEST_DYN_LINK)
elseif(boost_test_target_type STREQUAL UNKNOWN_LIBRARY)
  # Try inferring type if vcpkg is used
  if (DEFINED VCPKG_TARGET_TRIPLET)
    if (VCPKG_TARGET_TRIPLET EQUAL "x64-windows" OR VCPKG_TARGET_TRIPLET EQUAL "x86-windows" OR VCPKG_TARGET_TRIPLET EQUAL "arm64-osx-dynamic" OR VCPKG_TARGET_TRIPLET EQUAL "x64-osx-dynamic")
      message(STATUS "Boost::unit_test_framework looks to be a shared library based on vcpkg triplet ${VCPKG_TARGET_TRIPLET}. Adding BOOST_TEST_DYN_LINK")
      target_compile_definitions(rltest-onnx PRIVATE BOOST_TEST_DYN_LINK)
    endif()
  # If find_package is used then by default we're looking at a shared dependency unless Boost_USE_STATIC_LIBS was set.
  elseif(NOT Boost_USE_STATIC_LIBS)
    message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
    target_compile_definitions(rltest-onnx PRIVATE BOOST_TEST_DYN_LINK)
  endif()
endif()

if(RL_USE_UBSAN)
  # UBSan detects false positives due to test mocking
  target_compile_options(rltest-onnx PRIVATE -fno-sanitize=vptr)
  target_link_options(rltest-onnx PRIVATE -fno-sanitize=vptr)
endif()

add_test(NAME rltest-onnx 
  COMMAND $<TARGET_FILE:rltest-onnx> 
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})