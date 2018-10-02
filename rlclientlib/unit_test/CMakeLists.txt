
# If compiling on windows add the stdafx file
if(MSVC)
  set(msvc_stdafx "http_server/stdafx.cc")
endif()

add_executable(testlib
  ${msvc_stdafx}
  async_batcher_test.cc
  data_buffer_test.cc
  data_callback_test.cc
  err_callback_test.cc
  event_queue_test.cc
  event_test.cc
  eventhub_client_test.cc
  explore_test.cc
  factory_test.cc
  http_server/http_server.cc
  json_context_parse_test.cc
  live_model_test.cc
  main.cc
  mock_util.cc
  model_mgmt_test.cc
  object_pool_test.cc
  ranking_response_test.cc
  safe_vw_test.cc
  sleeper_test.cc
  status_builder_test.cc
  str_util_test.cc
  unit_test.vcxproj.filters
  watchdog_test.cc
)

set_target_properties(testlib PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Add the include directories from rlclientlib target for testing
target_include_directories(testlib PRIVATE $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>)

target_link_libraries(testlib PRIVATE
    rlclientlib fakeit Boost::unit_test_framework)

add_test(testlib testlib)
