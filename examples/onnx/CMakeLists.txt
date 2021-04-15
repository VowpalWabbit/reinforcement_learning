set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OnnxRuntime REQUIRED)

add_executable(onnx_example
  onnx_example.cc
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/mnist_data/)

set(ONNX_EXTENSION_TEST_RESOURCE_FILES
  ${CMAKE_CURRENT_LIST_DIR}/../../unit_test/extensions/onnx/mnist_data/mnist_model.onnx
)

add_custom_command(
  TARGET onnx_example POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${ONNX_EXTENSION_TEST_RESOURCE_FILES}
          ${CMAKE_CURRENT_BINARY_DIR}/mnist_data/
)

target_link_libraries(onnx_example PRIVATE rlclientlib-onnx)

add_custom_command(TARGET onnx_example POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  $<TARGET_FILE:onnxruntime>
  $<TARGET_FILE_DIR:onnx_example>
)
