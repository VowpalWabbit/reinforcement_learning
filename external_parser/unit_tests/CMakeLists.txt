find_package(Boost COMPONENTS unit_test_framework filesystem system program_options thread REQUIRED)

set(TEST_SOURCES
  test_vw_external_parser.cc
  test_vw_binary_parser.cc
  test_example_joiner.cc
  test_reward_functions.cc
  main.cc
  test_common.cc
  test_lru_dedup_cache.cc
  test_timestamp_helper.cc
  test_log_converter.cc
  test_skip_learn.cc
  test_metrics.cc
  test_client_and_enqueued_time.cc
)

add_executable(binary_parser_unit_tests ${TEST_SOURCES})

# Add the include directories from vw target for testing
target_include_directories(binary_parser_unit_tests
  PRIVATE
    $<TARGET_PROPERTY:vw_core,INCLUDE_DIRECTORIES>
)

target_link_libraries(binary_parser_unit_tests
  PRIVATE
    rl_binary_parser
    Boost::unit_test_framework
    Boost::system
    Boost::filesystem
)

# Automatically set BOOST_TEST_DYN_LINK if the dependency is shared.
get_target_property(boost_test_target_type Boost::unit_test_framework TYPE)
if (boost_test_target_type STREQUAL SHARED_LIBRARY)
  message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
  target_compile_definitions(binary_parser_unit_tests PRIVATE BOOST_TEST_DYN_LINK)
elseif(boost_test_target_type STREQUAL UNKNOWN_LIBRARY)
  # Try inferring type if vcpkg is used
  if (DEFINED VCPKG_TARGET_TRIPLET)
    if (VCPKG_TARGET_TRIPLET EQUAL "x64-windows" OR VCPKG_TARGET_TRIPLET EQUAL "x86-windows" OR VCPKG_TARGET_TRIPLET EQUAL "arm64-osx-dynamic" OR VCPKG_TARGET_TRIPLET EQUAL "x64-osx-dynamic")
      message(STATUS "Boost::unit_test_framework looks to be a shared library based on vcpkg triplet ${VCPKG_TARGET_TRIPLET}. Adding BOOST_TEST_DYN_LINK")
      target_compile_definitions(binary_parser_unit_tests PRIVATE BOOST_TEST_DYN_LINK)
    endif()
  # If find_package is used then by default we're looking at a shared dependency unless Boost_USE_STATIC_LIBS was set.
  elseif(NOT Boost_USE_STATIC_LIBS)
    message(STATUS "Boost::unit_test_framework looks to be a shared library. Adding BOOST_TEST_DYN_LINK")
    target_compile_definitions(binary_parser_unit_tests PRIVATE BOOST_TEST_DYN_LINK)
  endif()
endif()


add_test(NAME binary_parser_unit_tests COMMAND binary_parser_unit_tests -- ${CMAKE_CURRENT_LIST_DIR}/test_files/)
