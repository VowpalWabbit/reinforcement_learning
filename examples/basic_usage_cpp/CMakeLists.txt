add_executable(basic_usage_cpp.out
  basic_usage_cpp.cc
)

target_link_libraries(basic_usage_cpp.out PRIVATE rlclientlib)

if(RL_LINK_AZURE_LIBS)
  target_compile_definitions(basic_usage_cpp.out PRIVATE LINK_AZURE_LIBS)
  find_package(azure-identity-cpp CONFIG REQUIRED)
  target_link_libraries(basic_usage_cpp.out PRIVATE Azure::azure-identity)
endif()
