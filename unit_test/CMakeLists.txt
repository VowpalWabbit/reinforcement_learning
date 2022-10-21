set(TEST_SOURCES
  async_batcher_test.cc
  configuration_test.cc
  data_buffer_test.cc
  data_callback_test.cc
  dedup_test.cc
  err_callback_test.cc
  event_queue_test.cc
  explore_test.cc
  factory_test.cc
  fb_serializer_test.cc
  file_logger_test.cc
  header_auth_test.cc
  http_client_test.cc
  http_transport_client_test.cc
  json_context_parse_test.cc
  json_serializer_test.cc
  learning_mode_test.cc
  live_model_test.cc
  local_client_test.cc
  main.cc
  mock_http_client.cc
  mock_util.cc
  model_mgmt_test.cc
  moving_queue_test.cc
  multi_slot_response_detailed_test.cc
  multistep_test.cc
  object_pool_test.cc
  payload_serializer_test.cc
  preamble_test.cc
  ranking_response_test.cc
  safe_vw_test.cc
  #serializer.cc # won't compile
  sleeper_test.cc
  slot_ranking_test.cc
  status_builder_test.cc
  str_util_test.cc
  time_tests.cc
  trace_logger_test.cc
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

# Automatically set BOOST_TEST_DYN_LINK if the dependency is shared.
get_target_property(boost_test_target_type Boost::unit_test_framework TYPE)
if (boost_test_target_type STREQUAL SHARED_LIBRARY)
  message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
  target_compile_definitions(rltest PRIVATE BOOST_TEST_DYN_LINK)
elseif(boost_test_target_type STREQUAL UNKNOWN_LIBRARY)
  # Try inferring type if vcpkg is used
  if (DEFINED VCPKG_TARGET_TRIPLET)
    if (VCPKG_TARGET_TRIPLET EQUAL "x64-windows" OR VCPKG_TARGET_TRIPLET EQUAL "x86-windows" OR VCPKG_TARGET_TRIPLET EQUAL "arm64-osx-dynamic" OR VCPKG_TARGET_TRIPLET EQUAL "x64-osx-dynamic")
      message(STATUS "Boost::unit_test_framework looks to be a shared library based on vcpkg triplet ${VCPKG_TARGET_TRIPLET}. Adding BOOST_TEST_DYN_LINK")
      target_compile_definitions(rltest PRIVATE BOOST_TEST_DYN_LINK)
    endif()
  # If find_package is used then by default we're looking at a shared dependency unless Boost_USE_STATIC_LIBS was set.
  elseif(NOT Boost_USE_STATIC_LIBS)
    message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
    target_compile_definitions(rltest PRIVATE BOOST_TEST_DYN_LINK)
  endif()
endif()

if(RL_USE_UBSAN)
  # UBSan detects false positives due to test mocking
  target_compile_options(rltest PRIVATE -fno-sanitize=vptr)
  target_link_options(rltest PRIVATE -fno-sanitize=vptr)
endif()

target_link_libraries(rltest
  PRIVATE
    rlclientlib
    fakeit
    Boost::unit_test_framework
    Boost::system
)

add_test(NAME rltest COMMAND rltest)