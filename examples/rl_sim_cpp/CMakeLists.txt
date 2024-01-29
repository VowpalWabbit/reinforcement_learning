set(RL_SIM_SOURCES
  main.cc
  person.cc
  robot_joint.cc
  rl_sim.cc
)
if(RL_CXX_STANDARD GREATER_EQUAL 14 AND vw_USE_AZURE_FACTORIES)
  list(APPEND RL_SIM_SOURCES
    azure_credentials.cc
  )
endif()

add_executable(rl_sim_cpp.out
  ${RL_SIM_SOURCES}
)

target_link_libraries(rl_sim_cpp.out PRIVATE Boost::program_options rlclientlib)

if(RL_CXX_STANDARD GREATER_EQUAL 14 AND vw_USE_AZURE_FACTORIES)
  find_package(azure-identity-cpp CONFIG REQUIRED)
  target_link_libraries(rl_sim_cpp.out PRIVATE Azure::azure-identity)
endif()
