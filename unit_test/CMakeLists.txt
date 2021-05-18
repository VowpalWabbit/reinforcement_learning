set(TEST_SOURCES
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
    eventhub_client_test.cc
    mock_http_client.cc
    http_client_test.cc
  )

  if (WIN32)
    list(APPEND TEST_SOURCES
      model_mgmt_test_azure_win32.cc
    )
  endif()
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

#add_test(rltest rltest)

# Based on https://eb2.co/blog/2015/06/driving-boost.test-with-cmake/

# Read the sources, and identify tests

set(FOUND_TESTS "")
foreach(SOURCE_FILE_NAME IN LISTS TEST_SOURCES)
  file(READ "${SOURCE_FILE_NAME}" SOURCE_FILE_CONTENTS)
  string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\( *([A-Za-z_0-9]+) *\\)" 
         FOUND_TESTS_DELTA ${SOURCE_FILE_CONTENTS})
  list(APPEND FOUND_TESTS ${FOUND_TESTS_DELTA})
endforeach()

foreach(HIT ${FOUND_TESTS})
  string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\).*" "\\1" TEST_NAME ${HIT})
   
  add_test(NAME "rltest.${TEST_NAME}" 
    COMMAND $<TARGET_FILE:rltest>
    --run_test=${TEST_NAME} --catch_system_error=yes)
endforeach()
   