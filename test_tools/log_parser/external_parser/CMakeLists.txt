add_library(external_parser example_joiner.cc parse_example_external.cc ${CMAKE_CURRENT_SOURCE_DIR}/../../../rlclientlib/generated)

target_include_directories(external_parser PRIVATE $<TARGET_PROPERTY:vw,INCLUDE_DIRECTORIES> ${CMAKE_CURRENT_SOURCE_DIR}/../../../rlclientlib)
target_link_libraries(external_parser PRIVATE vw)