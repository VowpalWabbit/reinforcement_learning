add_library(rlclientlib
  api_status.cc
  console_tracer.cc
  error_callback_fn.cc
  factory_resolver.cc
  live_model_impl.cc
  live_model.cc
  async_batcher.cc
  logger/event_logger.cc
  logger/eventhub_client.cc
  model_mgmt/data_callback_fn.cc
  model_mgmt/model_downloader.cc
  model_mgmt/model_mgmt.cc
  model_mgmt/restapi_data_transport.cc
  ranking_event.cc
  ranking_response_impl.cc
  ranking_response.cc
  trace_logger.cc
  utility/config_utility.cc
  utility/configuration.cc
  utility/context_helper.cc
  utility/data_buffer.cc
  utility/str_util.cc
  utility/watchdog.cc
  vw_model/safe_vw.cc
  vw_model/vw_model.cc
)

set_target_properties(rlclientlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(rlclientlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rlclientlib PUBLIC vw cpprestsdk::cpprest Boost::system)

if(TURN_OFF_DEVIRTUALIZE)
  target_compile_options(rlclientlib PUBLIC -fno-devirtualize)
endif()

install(TARGETS rlclientlib
  INCLUDES DESTINATION rlclientlib
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)
