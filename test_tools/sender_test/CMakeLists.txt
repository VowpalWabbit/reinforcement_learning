add_executable(sender_test
  main.cc
  test_loop.cc
)

# Sender test uses internal headers from the rlclientlib target
target_include_directories(sender_test PRIVATE $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>)

target_link_libraries(sender_test PRIVATE Boost::program_options rlclientlib)
