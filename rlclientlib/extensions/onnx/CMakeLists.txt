set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OnnxRuntime REQUIRED)
find_package(cpprestsdk REQUIRED)

SET(ONNX_EXTENSION_SOURCES
  src/onnx_model.cc
  src/onnx_extension.cc
  src/onnx_input.cc
  src/tensor_parser.cc
)
  
SET(ONNX_EXTENSION_PUBLIC_HEADERS
  include/onnx_extension.h
)
  
SET(ONNX_EXTENSION_HEADERS
  src/onnx_model.h
  src/onnx_input.h
  src/tensor_parser.h
  src/tensor_notation.h
)

#source_group("Sources" FILES ${ONNX_EXTENSION_SOURCES})
#source_group("Public headers" FILES ${ONNX_EXTENSION_PUBLIC_HEADERS})
#source_group("Private headers" FILES ${ONNX_EXTENSION_HEADERS})

add_library(rlclientlib-onnx ${ONNX_EXTENSION_SOURCES} ${ONNX_EXTENSION_PUBLIC_HEADERS} ${ONNX_EXTENSION_PRIVATE_HEADERS})

set_target_properties(rlclientlib-onnx PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(rlclientlib-onnx
                           PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/src
                           ${ONNXRUNTIME_INCLUDE_DIRS}
                           )

target_link_libraries(rlclientlib-onnx PUBLIC rlclientlib onnxruntime)