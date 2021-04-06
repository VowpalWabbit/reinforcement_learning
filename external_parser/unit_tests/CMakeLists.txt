find_package(Boost COMPONENTS unit_test_framework filesystem system program_options thread REQUIRED)

set(TEST_SOURCES
  test_vw_external_parser.cc
  test_vw_binary_parser.cc
  test_example_joiner.cc
  main.cc
  test_common.cc
  test_lru_dedup_cache.cc
)

add_executable(binary_parser_unit_tests ${TEST_SOURCES})

# Add the include directories from vw target for testing
target_include_directories(binary_parser_unit_tests 
  PRIVATE 
    $<TARGET_PROPERTY:vw,INCLUDE_DIRECTORIES>
)

target_link_libraries(binary_parser_unit_tests 
  PRIVATE
    vw
    Boost::unit_test_framework 
    Boost::system
    Boost::filesystem
)

add_test(binary_parser_unit_tests binary_parser_unit_tests)
