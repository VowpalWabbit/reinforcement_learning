
# If compiling on windows add the stdafx file
add_executable(rltest
  async_batcher_test.cc
  configuration_test.cc
  data_buffer_test.cc
  data_callback_test.cc
  err_callback_test.cc
  event_queue_test.cc
  event_test.cc
  eventhub_client_test.cc
  explore_test.cc
  factory_test.cc
  mock_http_server.cc
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

# Devirtualization is an optimization that changes the vtable if the compiler decides a function
# doesn't need to be virtual. This is incompatible with the mocking framework used in testing as it
# makes the vtable structure unpredictable
if(MSVC)
  # MSVC doesn't optimize aggresively enough for this bug to be an issue right now.
else()
  target_compile_options(rltest PRIVATE -fno-devirtualize)
endif()

# Add the include directories from rlclientlib target for testing
target_include_directories(rltest PRIVATE $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>)

target_link_libraries(rltest PRIVATE
  rlclientlib fakeit Boost::unit_test_framework Boost::system)

add_test(rltest rltest)
