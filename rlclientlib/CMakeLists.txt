add_subdirectory(unit_test)

add_library(rlclientlib
  src/api_status.cc
  src/console_tracer.cc
  src/error_callback_fn.cc
  src/factory_resolver.cc
  src/live_model_impl.cc
  src/live_model.cc
  src/logger/async_batcher.cc
  src/logger/event_logger.cc
  src/logger/eventhub_client.cc
  src/model_mgmt/data_callback_fn.cc
  src/model_mgmt/model_downloader.cc
  src/model_mgmt/model_mgmt.cc
  src/model_mgmt/restapi_data_transport.cc
  src/ranking_event.cc
  src/ranking_response_impl.cc
  src/ranking_response.cc
  src/trace_logger.cc
  src/utility/config_utility.cc
  src/utility/configuration.cc
  src/utility/context_helper.cc
  src/utility/data_buffer.cc
  src/utility/str_util.cc
  src/utility/watchdog.cc
  src/vw_model/safe_vw.cc
  src/vw_model/vw_model.cc
)

# TODO Ideally most of these options should come as public dependencies on a VW cmake project

target_compile_definitions(rlclientlib PUBLIC _FILE_OFFSET_BITS=64 NDEBUG __extern_always_inline=inline)
set_target_properties(rlclientlib PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  target_compile_options(rlclientlib PRIVATE /W4)
  target_compile_options(rlclientlib PUBLIC /MP -D__SSE2__)
else()
  target_compile_options(rlclientlib PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(rlclientlib PUBLIC -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse)
endif()

target_include_directories(rlclientlib PUBLIC include PRIVATE src ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rlclientlib PRIVATE vw allreduce cpprestsdk::cpprest Boost::system Boost::program_options dl z rapidjson)

# TODO complete module installation
