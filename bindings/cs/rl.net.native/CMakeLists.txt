add_library(rl.net.native SHARED
  rl.net.api_status.cc
  rl.net.config.cc
  rl.net.live_model.cc
  rl.net.ranking_response.cc
  rl.net.decision_response.cc
  rl.net.slates_response.cc
  binding_tracer.cc
)

if(MSVC)
  target_compile_options(rl.net.native PRIVATE /guard:cf)
endif()

set_target_properties(rl.net.native PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(rl.net.native PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rl.net.native PUBLIC rlclientlib)
