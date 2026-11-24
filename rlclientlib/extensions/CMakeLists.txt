if (rlclientlib_BUILD_ONNXRUNTIME_EXTENSION)
  message("Building RLClientLib Extension: OnnxRuntime")
  add_subdirectory(onnx)
endif()