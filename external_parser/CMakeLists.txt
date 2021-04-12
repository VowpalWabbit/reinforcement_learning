cmake_minimum_required(VERSION 3.5)
project(vw_binary_parser)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG") 
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING INTERNAL FORCE)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug, Release" FORCE)

include(ProcessorCount)
ProcessorCount(NumProcessors)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nprocs.txt ${NumProcessors})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules/")
find_package(flatbuffers REQUIRED)

set(RL_FLAT_BUFFER_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/DedupInfo.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/Metadata.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/OutcomeEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/CbEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/CaEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/FileFormat.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/MultiSlotEvent.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/Event.fbs"
  "${CMAKE_CURRENT_SOURCE_DIR}/../rlclientlib/schema/v2/LearningModeType.fbs" )

set(BUILD_EXTERNAL_PARSER "Build external parser" ON)
set(WARNINGS OFF CACHE BOOL "")
set(BUILD_TESTS OFF CACHE BOOL "") # vw tests off
build_flatbuffers("${RL_FLAT_BUFFER_FILES}" "" fbgen "" "${CMAKE_CURRENT_SOURCE_DIR}/generated/v2/" "" "")

set(external_parser_headers ${CMAKE_CURRENT_SOURCE_DIR}/lru_dedup_cache.h
  ${CMAKE_CURRENT_SOURCE_DIR}/example_joiner.h
  ${CMAKE_CURRENT_SOURCE_DIR}/parse_example_external.h
  ${CMAKE_CURRENT_SOURCE_DIR}/parse_example_binary.h
)
set(external_parser_sources ${CMAKE_CURRENT_SOURCE_DIR}/lru_dedup_cache.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/example_joiner.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parse_example_external.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/parse_example_binary.cc
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/zstd/build/cmake ${CMAKE_CURRENT_BINARY_DIR}/vw_binary_parser/zstd EXCLUDE_FROM_ALL)
set(EXTERNAL_PARSER_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ext_libs/zstd/lib/)
set(EXTERNAL_PARSER_LIBS libzstd_static)

add_subdirectory(${CMAKE_SOURCE_DIR}/../ext_libs/vowpal_wabbit ${CMAKE_CURRENT_BINARY_DIR}/vw_binary_parser)
add_dependencies(vw fbgen)

enable_testing()
add_subdirectory(unit_tests)
