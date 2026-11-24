set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../../cmake/Modules/")

set(RL_SIM_SOURCES
  main.cc
  person.cc
  robot_joint.cc
  rl_sim.cc
)

add_executable(rl_sim_cpp.out
  ${RL_SIM_SOURCES}
)

target_link_libraries(rl_sim_cpp.out PRIVATE Boost::program_options rlclientlib)

if(RL_LINK_AZURE_LIBS)
  target_compile_definitions(rl_sim_cpp.out PRIVATE LINK_AZURE_LIBS)
  find_package(azure-identity-cpp CONFIG REQUIRED)
  target_link_libraries(rl_sim_cpp.out PRIVATE Azure::azure-identity)
endif()
