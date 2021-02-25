add_executable(binary_parser
  binary_parser/parser.cc
  binary_parser/example_joiner.cc
  binary_parser/joined_log_parser.cc
)

target_include_directories(binary_parser PRIVATE $<TARGET_PROPERTY:rlclientlib,INCLUDE_DIRECTORIES>)
target_link_libraries(binary_parser PRIVATE Boost::program_options rlclientlib)

