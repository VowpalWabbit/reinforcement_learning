add_subdirectory(basic_usage_cpp)
add_subdirectory(local_loop)
add_subdirectory(override_interface)
add_subdirectory(rl_sim_cpp)
add_subdirectory(test_cpp)

if (rlclientlib_BUILD_ONNXRUNTIME_EXTENSION)
  add_subdirectory(onnx)
endif()
