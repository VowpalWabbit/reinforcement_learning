add_executable(rl_sim_cpp.out
  main.cc
  person.cc
  robot_joint.cc
  rl_sim.cc
  local_loop.cc
local_loop.h
)

set(RL_EXTERNAL_PARSER_OTHER_DEPS OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../external_parser ${CMAKE_CURRENT_BINARY_DIR}/external_parser)

target_link_libraries(rl_sim_cpp.out PRIVATE Boost::program_options rlclientlib vw_core rl_binary_parser)
