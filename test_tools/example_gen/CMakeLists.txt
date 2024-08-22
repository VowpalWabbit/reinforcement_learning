add_executable(example_gen
  example_gen.cc
)
target_link_libraries(example_gen PRIVATE Boost::program_options rlclientlib)

if(RL_LINK_AZURE_LIBS)
  target_compile_definitions(example_gen PRIVATE LINK_AZURE_LIBS)
  find_package(azure-identity-cpp CONFIG REQUIRED)
  target_link_libraries(example_gen PRIVATE Azure::azure-identity)
endif()
