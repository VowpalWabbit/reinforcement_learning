if(RL_BUILD_PYTHON)
  pybind11_add_module(rl_client py_api.cc)
  target_link_libraries(rl_client PRIVATE rlclientlib)
endif()
