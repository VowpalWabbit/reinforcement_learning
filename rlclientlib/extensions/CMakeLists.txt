# Flags for building optional extensions
set(rlclientlib_BUILD_ONNXRUNTIME_EXTENSION OFF CACHE BOOL "Build OnnxRuntime Inference Extension")

if (rlclientlib_BUILD_ONNXRUNTIME_EXTENSION)
  message("Building RLClientLib Extension: OnnxRuntime")
  add_subdirectory(onnx)
endif()