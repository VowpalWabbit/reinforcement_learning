add_executable(override_interface.out
  override_interface.cc
)

target_link_libraries(override_interface.out PRIVATE rlclientlib)

if(RL_LINK_AZURE_LIBS)
  target_compile_definitions(override_interface.out PRIVATE LINK_AZURE_LIBS)
  find_package(azure-identity-cpp CONFIG REQUIRED)
  target_link_libraries(override_interface.out PRIVATE Azure::azure-identity)
endif()
