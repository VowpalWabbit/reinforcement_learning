add_executable(rl_sim_cpp.out
  main.cc
  person.cc
  robot_joint.cc
  rl_sim.cc
)

target_link_libraries(rl_sim_cpp.out PRIVATE Boost::program_options rlclientlib)
