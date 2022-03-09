add_executable(rl_test.out
  main.cc
  options.cc
  experiment_controller.cc
  test_data_provider.cc
  test_loop.cc
)

# TODO remove internal header dependency of this example
target_include_directories(rl_test.out PRIVATE $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>)

target_link_libraries(rl_test.out PRIVATE Boost::program_options rlclientlib)
