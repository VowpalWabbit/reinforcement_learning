add_executable(example_gen
  example_gen.cc
)
target_link_libraries(example_gen PRIVATE Boost::program_options rlclientlib)

