set(TEST_SOURCES
  apim_http_auth_test.cc
  async_batcher_test.cc
  configuration_test.cc
  data_buffer_test.cc
  data_callback_test.cc
  err_callback_test.cc
  dedup_test.cc
  event_queue_test.cc
  explore_test.cc
  factory_test.cc
  fb_serializer_test.cc
  json_context_parse_test.cc
  learning_mode_test.cc
  live_model_test.cc
  main.cc
  mock_util.cc
  model_mgmt_test.cc
  object_pool_test.cc
  payload_serializer_test.cc
  ranking_response_test.cc
  safe_vw_test.cc
  sleeper_test.cc
  status_builder_test.cc
  str_util_test.cc
  unit_test.vcxproj.filters
  watchdog_test.cc
)

if (vw_USE_AZURE_FACTORIES)
  list(APPEND TEST_SOURCES
    http_transport_client_test.cc
    mock_http_client.cc
    http_client_test.cc
  )
endif()

# If compiling on windows add the stdafx file
add_executable(rltest ${TEST_SOURCES})

# Add the include directories from rlclientlib target for testing
target_include_directories(rltest 
  PRIVATE 
    $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>
)

if(vw_USE_AZURE_FACTORIES)
  target_compile_definitions(rltest PRIVATE USE_AZURE_FACTORIES)
endif()

target_link_libraries(rltest 
  PRIVATE
    rlclientlib
    fakeit 
    Boost::unit_test_framework 
    Boost::system
)

add_test(rltest rltest)