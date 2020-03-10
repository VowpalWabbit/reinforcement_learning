set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(FlatBuffers REQUIRED)

set(RL_FLAT_BUFFER_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/Metadata.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/OutcomeEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/RankingEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/DecisionRankingEvent.fbs" )
build_flatbuffers("${RL_FLAT_BUFFER_FILES}" "" fbgenerator "" "${CMAKE_CURRENT_SOURCE_DIR}/generated" "" "")

set(PROJECT_SOURCES
  api_status.cc
  azure_factories.cc
  console_tracer.cc
  decision_response.cc
  error_callback_fn.cc
  factory_resolver.cc
  live_model_impl.cc
  live_model.cc
  learning_mode.cc
  time_helper.cc
  logger/async_batcher.cc
  logger/event_logger.cc
  logger/eventhub_client.cc
  logger/flatbuffer_allocator.cc
  logger/preamble.cc
  logger/preamble_sender.cc
  logger/endian.cc
  logger/file/file_logger.cc
  model_mgmt/data_callback_fn.cc
  model_mgmt/empty_data_transport.cc
  model_mgmt/model_downloader.cc
  model_mgmt/model_mgmt.cc
  model_mgmt/restapi_data_transport.cc
  model_mgmt/file_model_loader.cc
  ranking_event.cc
  ranking_response.cc
  sampling.cc
  trace_logger.cc
  utility/stl_container_adapter.cc
  utility/config_utility.cc
  utility/configuration.cc
  utility/context_helper.cc
  utility/data_buffer.cc
  utility/data_buffer_streambuf.cc
  utility/http_authorization.cc
  utility/http_client.cc
  utility/http_helper.cc
  utility/str_util.cc
  utility/watchdog.cc
  vw_model/pdf_model.cc
  vw_model/safe_vw.cc
  vw_model/vw_model.cc
)

set(PROJECT_PUBLIC_HEADERS
  ../include/action_flags.h
  ../include/api_status.h
  ../include/config_utility.h
  ../include/configuration.h
  ../include/constants.h
  ../include/container_iterator.h
  ../include/decision_response.h
  ../include/err_constants.h
  ../include/errors_data.h
  ../include/factory_resolver.h
  ../include/future_compat.h
  ../include/live_model.h
  ../include/model_mgmt.h
  ../include/object_factory.h
  ../include/personalization.h
  ../include/ranking_response.h
  ../include/sender.h
  ../include/str_util.h
  ../include/trace_logger.h
  ../include/data_buffer.h
)

set(PROJECT_PRIVATE_HEADERS
  azure_factories.h
  console_tracer.h
  error_callback_fn.h
  live_model_impl.h
  logger/async_batcher.h
  logger/event_logger.h
  logger/eventhub_client.h
  model_mgmt/data_callback_fn.h
  model_mgmt/empty_data_transport.h
  model_mgmt/model_downloader.h
  model_mgmt/restapi_data_transport.h
  model_mgmt/file_model_loader.h
  moving_queue.h
  ranking_event.h
  sampling.h
  serialization/fb_serializer.h
  serialization/json_serializer.h
  utility/context_helper.h
  utility/http_authorization.h
  utility/http_client.h
  utility/http_helper.h
  utility/interruptable_sleeper.h
  utility/object_pool.h
  utility/periodic_background_proc.h
  utility/watchdog.h
  vw_model/pdf_model.h
  vw_model/safe_vw.h
  vw_model/vw_model.h
)

source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Public headers" FILES ${PROJECT_PUBLIC_HEADERS})
source_group("Private headers" FILES ${PROJECT_PRIVATE_HEADERS})

add_library(rlclientlib ${PROJECT_SOURCES} ${PROJECT_PUBLIC_HEADERS} ${PROJECT_PRIVATE_HEADERS})
add_dependencies(rlclientlib fbgenerator)
set_target_properties(rlclientlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories( rlclientlib
                            PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}/../include
                            PRIVATE
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${FLATBUFFERS_INCLUDE_DIRS}
                            ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/date
                            )
target_link_libraries(rlclientlib PUBLIC Boost::system vw OpenSSL::SSL OpenSSL::Crypto cpprestsdk::cpprest PRIVATE RapidJSON)
# Consuming Boost uuid requires BCrypt, normally this is automatically linked but vcpkg turns this feature off.
if(WIN32)
  target_link_libraries(rlclientlib PUBLIC bcrypt)
endif()

# On MacOS linking fails unless we explicitly add Boost::thread. It seems like CppRestSDK isn't exporting dependencies properly.
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_link_libraries(rlclientlib PUBLIC Boost::thread)
endif()

install(TARGETS rlclientlib
  INCLUDES DESTINATION rlclientlib
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)
