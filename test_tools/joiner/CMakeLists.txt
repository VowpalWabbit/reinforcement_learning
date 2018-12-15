add_executable(joiner.out
  main.cc
  text_converter.cc
)

target_link_libraries(rl_sim_cpp.out PRIVATE Boost::program_options rlclientlib)
