cmake_minimum_required(VERSION 3.5)
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()
project(vw_binary_parser)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(ProcessorCount)
ProcessorCount(NumProcessors)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nprocs.txt ${NumProcessors})

option(STATIC_LINK_BINARY_PARSER "Link VW binary parser executable statically. Off by default." OFF)
option(BUILD_BINARY_PARSER_TESTS "Build and enable tests." ON)

if(WIN32 AND (STATIC_LINK_BINARY_PARSER))
  message(FATAL_ERROR "Unsupported option enabled on Windows build")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/../cmake/Modules/")

# Dependencies
# ------------

# First try to find the config version. Newer, used by vcpkg etc
find_package(Flatbuffers CONFIG QUIET)
if(FLATBUFFERS_FOUND)
  get_property(flatc_location TARGET flatbuffers::flatc PROPERTY LOCATION)
else()
  # Fallback to the old version
  find_package(Flatbuffers MODULE REQUIRED)
  set(flatc_location ${FLATBUFFERS_FLATC_EXECUTABLE})
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../ext_libs/zstd/build/cmake ${CMAKE_CURRENT_BINARY_DIR}/vw_binary_parser/zstd EXCLUDE_FROM_ALL)

# set vw cmake flags
# This is not longer using the injected method, but implemented in this repo just consuming vw as a lib
set(BUILD_EXTERNAL_PARSER OFF CACHE BOOL "")
set(BUILD_FLATBUFFERS OFF CACHE BOOL "")
set(DO_NOT_BUILD_VW_C_WRAPPER OFF CACHE BOOL "")
set(BUILD_JAVA OFF CACHE BOOL "")
set(BUILD_PYTHON OFF CACHE BOOL "")
set(BUILD_EXPERIMENTAL_BINDING OFF CACHE BOOL "")
set(BUILD_TESTING OFF CACHE BOOL "")

if(STATIC_LINK_BINARY_PARSER)
  set(STATIC_LINK_VW ON CACHE BOOL "" FORCE)
endif()
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../ext_libs/vowpal_wabbit ${CMAKE_CURRENT_BINARY_DIR}/vw_binary_parser)

# Flatbuffer generation
# ---------------------

include(FlatbufferUtils)
set(RL_FLAT_BUFFER_FILES
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/DedupInfo.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/Metadata.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/OutcomeEvent.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/CbEvent.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/CaEvent.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/FileFormat.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/MultiSlotEvent.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/Event.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/LearningModeType.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/ProblemType.fbs"
  "${CMAKE_CURRENT_LIST_DIR}/../rlclientlib/schema/v2/MultiStepEvent.fbs"
)

add_flatbuffer_schema(
  TARGET fbgen
  SCHEMAS ${RL_FLAT_BUFFER_FILES}
  OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated/v2/
  FLATC_EXE ${flatc_location}
)

# Binary parser lib and exe
# -------------------------

set(binary_parser_headers
  ${CMAKE_CURRENT_LIST_DIR}/event_processors/timestamp_helper.h
  ${CMAKE_CURRENT_LIST_DIR}/joiners/example_joiner.h
  ${CMAKE_CURRENT_LIST_DIR}/joiners/i_joiner.h
  ${CMAKE_CURRENT_LIST_DIR}/joiners/multistep_example_joiner.h
  ${CMAKE_CURRENT_LIST_DIR}/log_converter.h
  ${CMAKE_CURRENT_LIST_DIR}/lru_dedup_cache.h
  ${CMAKE_CURRENT_LIST_DIR}/parse_example_binary.h
  ${CMAKE_CURRENT_LIST_DIR}/parse_example_converter.h
  ${CMAKE_CURRENT_LIST_DIR}/parse_example_external.h
  ${CMAKE_CURRENT_LIST_DIR}/utils.h
)
set(binary_parser_sources
  ${CMAKE_CURRENT_LIST_DIR}/event_processors/timestamp_helper.cc
  ${CMAKE_CURRENT_LIST_DIR}/joiners/example_joiner.cc
  ${CMAKE_CURRENT_LIST_DIR}/joiners/multistep_example_joiner.cc
  ${CMAKE_CURRENT_LIST_DIR}/log_converter.cc
  ${CMAKE_CURRENT_LIST_DIR}/lru_dedup_cache.cc
  ${CMAKE_CURRENT_LIST_DIR}/parse_example_binary.cc
  ${CMAKE_CURRENT_LIST_DIR}/parse_example_converter.cc
  ${CMAKE_CURRENT_LIST_DIR}/parse_example_external.cc
  ${CMAKE_CURRENT_LIST_DIR}/utils.cc
)

add_library(rl_binary_parser STATIC ${binary_parser_headers} ${binary_parser_sources})
target_link_libraries(rl_binary_parser PUBLIC vw_core PRIVATE libzstd_static)
target_include_directories(rl_binary_parser
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/../ext_libs/zstd/lib/
    ${CMAKE_CURRENT_LIST_DIR}/../ext_libs/date/
    ${FLATBUFFERS_INCLUDE_DIR}
  )
add_dependencies(rl_binary_parser fbgen)

add_executable(rl_binary_parser_bin main.cc)
target_link_libraries(rl_binary_parser_bin PUBLIC rl_binary_parser)
set_target_properties(rl_binary_parser_bin PROPERTIES OUTPUT_NAME "vw")

if(STATIC_LINK_BINARY_PARSER AND NOT APPLE)
  target_link_libraries(rl_binary_parser_bin PRIVATE -static)
endif()


# Tests
# -----

if (BUILD_BINARY_PARSER_TESTS)
  enable_testing()
  add_subdirectory(unit_tests)
endif()
