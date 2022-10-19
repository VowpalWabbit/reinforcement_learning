add_executable(rl_sim_cpp.out
  main.cc
  person.cc
  robot_joint.cc
  rl_sim.cc
  local_loop.cc
  local_loop.h
)

target_link_libraries(rl_sim_cpp.out PRIVATE Boost::program_options rlclientlib vw_core rl_binary_parser)
