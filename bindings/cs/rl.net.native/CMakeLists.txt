set(rl_net_native_SOURCES
  binding_sender.cc
  binding_tracer.cc
  rl.net.api_status.cc
  rl.net.buffer.cc
  rl.net.config.cc
  rl.net.continuous_action_response.cc
  rl.net.decision_response.cc
  rl.net.factory_context.cc
  rl.net.live_model.cc
  rl.net.native.cc
  rl.net.ranking_response.cc
  rl.net.multi_slot_response.cc
  rl.net.multi_slot_response_detailed.cc
  rl.net.slot_ranking.cc
)

set(rl_net_native_HEADERS
  binding_sender.h
  binding_tracer.h
  rl.net.api_status.h
  rl.net.buffer.h
  rl.net.config.h
  rl.net.continuous_action_response.h
  rl.net.decision_response.h
  rl.net.factory_context.h
  rl.net.live_model.h
  rl.net.native.h
  rl.net.ranking_response.h
  rl.net.multi_slot_response.h
  rl.net.multi_slot_response_detailed.h
  rl.net.slot_ranking.h
)

source_group("Sources" FILES ${rl_net_native_SOURCES})
source_group("Headers" FILES ${rl_net_native_HEADERS})

add_library(rlnetnative SHARED
  ${rl_net_native_SOURCES} ${rl_net_native_HEADERS}
)

if(MSVC)
  target_compile_options(rlnetnative PRIVATE /guard:cf)
endif()

set_target_properties(rlnetnative PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(rlnetnative PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rlnetnative PUBLIC rlclientlib)

# Enable warnings as errors, crossplatform
target_compile_options(rlnetnative PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Werror>
     $<$<CXX_COMPILER_ID:MSVC>:
          /WX>)
