set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(flatbuffers REQUIRED)

set(RL_FLAT_BUFFER_FILES_V1
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v1/Metadata.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v1/OutcomeEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v1/RankingEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v1/DecisionRankingEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v1/SlatesEvent.fbs" )

set(RL_FLAT_BUFFER_FILES_V2
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v2/DedupInfo.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v2/Metadata.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v2/OutcomeEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v2/CbEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v2/CaEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v2/MultiSlotEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v2/MultiStepEvent.fbs"  
  "${CMAKE_CURRENT_SOURCE_DIR}/schema/v2/Event.fbs" )

build_flatbuffers("${RL_FLAT_BUFFER_FILES_V1}" "" fbgenerator_v1 "" "${CMAKE_CURRENT_SOURCE_DIR}/generated/v1/" "" "")
build_flatbuffers("${RL_FLAT_BUFFER_FILES_V2}" "" fbgenerator_v2 "" "${CMAKE_CURRENT_SOURCE_DIR}/generated/v2/" "" "")

set(PROJECT_SOURCES
  constants.cc
  api_status.cc
  console_tracer.cc
  continuous_action_response.cc
  decision_response.cc
  dedup.cc
  error_callback_fn.cc
  factory_resolver.cc
  live_model_impl.cc
  live_model.cc
  learning_mode.cc
  time_helper.cc
  logger/event_logger.cc
  logger/flatbuffer_allocator.cc
  logger/logger_facade.cc
  logger/logger_extensions.cc
  logger/preamble.cc
  logger/preamble_sender.cc
  logger/endian.cc
  logger/file/file_logger.cc
  model_mgmt/data_callback_fn.cc
  model_mgmt/empty_data_transport.cc
  model_mgmt/model_downloader.cc
  model_mgmt/model_mgmt.cc
  model_mgmt/file_model_loader.cc
  multistep.cc
  generic_event.cc
  ranking_event.cc
  ranking_response.cc
  sampling.cc
  multi_slot_response_detailed.cc
  slot_ranking.cc
  serialization/payload_serializer.cc
  multi_slot_response.cc
  trace_logger.cc
  utility/stl_container_adapter.cc
  utility/config_helper.cc
  utility/config_utility.cc
  utility/configuration.cc
  utility/context_helper.cc
  utility/data_buffer.cc
  utility/data_buffer_streambuf.cc
  utility/str_util.cc
  utility/watchdog.cc
  vw_model/pdf_model.cc
  vw_model/safe_vw.cc
  vw_model/vw_model.cc
)

if(vw_USE_AZURE_FACTORIES)
  list(APPEND PROJECT_SOURCES
    azure_factories.cc
    logger/eventhub_client.cc
    model_mgmt/restapi_data_transport.cc
    utility/http_client.cc
    utility/http_helper.cc
    utility/http_authorization.cc
  )
endif()

set(PROJECT_PUBLIC_HEADERS
  ../include/action_flags.h
  ../include/api_status.h
  ../include/config_utility.h
  ../include/configuration.h
  ../include/constants.h
  ../include/container_iterator.h
  ../include/continuous_action_response.h
  ../include/decision_response.h
  ../include/err_constants.h
  ../include/error_callback_fn.h
  ../include/errors_data.h
  ../include/factory_resolver.h
  ../include/future_compat.h
  ../include/live_model.h
  ../include/model_mgmt.h
  ../include/multistep.h
  ../include/object_factory.h
  ../include/personalization.h
  ../include/ranking_response.h
  ../include/sender.h
  ../include/multi_slot_response.h
  ../include/str_util.h
  ../include/trace_logger.h
  ../include/data_buffer.h
  ../include/slot_ranking.h
  ../include/multi_slot_response_detailed.h
)

set(PROJECT_PRIVATE_HEADERS
  console_tracer.h
  dedup.h
  live_model_impl.h
  logger/async_batcher.h
  logger/event_logger.h
  logger/logger_facade.h
  model_mgmt/data_callback_fn.h
  model_mgmt/empty_data_transport.h
  model_mgmt/model_downloader.h
  model_mgmt/file_model_loader.h
  moving_queue.h
  generic_event.h
  ranking_event.h
  rl_string_view.h
  sampling.h
  serialization/fb_serializer.h
  serialization/json_serializer.h
  utility/context_helper.h
  utility/interruptable_sleeper.h
  utility/object_pool.h
  utility/periodic_background_proc.h
  utility/watchdog.h
  utility/config_helper.h
  vw_model/pdf_model.h
  vw_model/safe_vw.h
  vw_model/vw_model.h
)

if(vw_USE_AZURE_FACTORIES)
  list(APPEND PROJECT_PRIVATE_HEADERS
    azure_factories.h
    logger/eventhub_client.h
    model_mgmt/restapi_data_transport.h
    utility/http_client.h
    utility/http_helper.h
    utility/http_authorization.h
  )
endif()

source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Public headers" FILES ${PROJECT_PUBLIC_HEADERS})
source_group("Private headers" FILES ${PROJECT_PRIVATE_HEADERS})

add_library(rlclientlib ${PROJECT_SOURCES} ${PROJECT_PUBLIC_HEADERS} ${PROJECT_PRIVATE_HEADERS})
add_dependencies(rlclientlib fbgenerator_v1 fbgenerator_v2)

if(vw_USE_AZURE_FACTORIES)
  target_compile_definitions(rlclientlib PRIVATE USE_AZURE_FACTORIES)
endif()

set_target_properties(rlclientlib PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(USE_ZSTD)
  target_compile_definitions(rlclientlib PRIVATE USE_ZSTD)
  target_link_libraries(rlclientlib PRIVATE libzstd_static)
  target_include_directories(rlclientlib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/zstd/lib/)
endif()

target_include_directories( rlclientlib
                            PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}/../include
                            PRIVATE
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${FLATBUFFERS_INCLUDE_DIRS}
                            ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/date
                            )
target_link_libraries(rlclientlib PUBLIC Boost::system vw OpenSSL::SSL OpenSSL::Crypto PRIVATE RapidJSON)

if (vw_USE_AZURE_FACTORIES)
  target_link_libraries(rlclientlib PUBLIC cpprestsdk::cpprest)
endif()

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
