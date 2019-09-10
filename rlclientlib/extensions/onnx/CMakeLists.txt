set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OnnxRuntime REQUIRED)
find_package(cpprestsdk REQUIRED)

SET(ONNX_EXTENSION_SOURCES
  src/onnx_model.cc
  src/onnx_extension.cc
  src/tensor_notation.cc
)
  
SET(ONNX_EXTENSION_PUBLIC_HEADERS
  include/onnx_extension.h
  #src/onnxrt_representation.h
)
  
SET(ONNX_EXTENSION_HEADERS
  src/onnx_model.h
  #src/onnxrt_type_mapping.h
  src/tensor_notation.h
)

#source_group("Sources" FILES ${ONNX_EXTENSION_SOURCES})
#source_group("Public headers" FILES ${ONNX_EXTENSION_PUBLIC_HEADERS})
#source_group("Private headers" FILES ${ONNX_EXTENSION_HEADERS})

add_library(rlclientlib-onnx STATIC ${ONNX_EXTENSION_SOURCES} ${ONNX_EXTENSION_PUBLIC_HEADERS} ${ONNX_EXTENSION_PRIVATE_HEADERS})

set_target_properties(rlclientlib-onnx PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_dependencies(rlclientlib-onnx rlclientlib)

target_include_directories( rlclientlib-onnx
                            PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}/include
                            PRIVATE
                            $<TARGET_PROPERTY:rlclientlib,INTERFACE_INCLUDE_DIRECTORIES>
                            ${CMAKE_CURRENT_SOURCE_DIR}/src
                            ${ONNXRUNTIME_INCLUDE_DIRS}
                            ${CPPRESTSDK_INCLUDE_DIRS}
                            )

target_link_libraries(rlclientlib-onnx PUBLIC rlclientlib onnxruntime)

if(TURN_OFF_DEVIRTUALIZE)
  target_compile_options(rlclientlib-onnx PUBLIC -fno-devirtualize)
endif()