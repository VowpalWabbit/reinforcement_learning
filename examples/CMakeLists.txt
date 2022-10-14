add_subdirectory(basic_usage_cpp)
add_subdirectory(override_interface)
add_subdirectory(test_cpp)

if(RL_BUILD_EXTERNAL_PARSER)
  add_subdirectory(rl_sim_cpp)
endif()

if (rlclientlib_BUILD_ONNXRUNTIME_EXTENSION)
  add_subdirectory(onnx)
endif()
