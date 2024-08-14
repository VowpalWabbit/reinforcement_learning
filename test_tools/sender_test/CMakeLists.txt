add_executable(sender_test
  main.cc
  test_loop.cc
)

# Sender test uses internal headers from the rlclientlib target
target_include_directories(sender_test PRIVATE $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>)

target_link_libraries(sender_test PRIVATE Boost::program_options rlclientlib)

if(RL_LINK_AZURE_LIBS)
  target_compile_definitions(sender_test PRIVATE LINK_AZURE_LIBS)
  find_package(azure-identity-cpp CONFIG REQUIRED)
  target_link_libraries(sender_test PRIVATE Azure::azure-identity)
endif()
