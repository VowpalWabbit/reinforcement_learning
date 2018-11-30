add_executable(rl_sim_cpp.out
  main.cc
  person.cc
  rl_sim.cc
  simulation_stats.cc
)

target_link_libraries(rl_sim_cpp.out PRIVATE Boost::program_options rlclientlib)
