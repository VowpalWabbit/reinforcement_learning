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
set(EXTERNAL_PARSER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VW_INSTALL OFF CACHE BOOL "")
set(BUILD_TESTS OFF CACHE BOOL "")
set(BUILD_JAVA OFF CACHE BOOL "")
set(BUILD_PYTHON OFF CACHE BOOL "")
set(BUILD_DOCS OFF CACHE BOOL "")
set(WARNINGS OFF CACHE BOOL "")
set(BUILD_FLATBUFFERS OFF CACHE BOOL "")
build_flatbuffers("${RL_FLAT_BUFFER_FILES}" "" fbgen "" "${CMAKE_CURRENT_SOURCE_DIR}/generated/v2/" "" "")

set(external_parser_headers ${CMAKE_CURRENT_SOURCE_DIR}/example_joiner.h ${CMAKE_CURRENT_SOURCE_DIR}/parse_example_external.h)
set(external_parser_sources ${CMAKE_CURRENT_SOURCE_DIR}/example_joiner.cc ${CMAKE_CURRENT_SOURCE_DIR}/parse_example_external.cc)

add_subdirectory(${CMAKE_SOURCE_DIR}/ext_libs/vowpal_wabbit ${CMAKE_CURRENT_BINARY_DIR}/vw_with_binary_parser)