add_executable(binary_parser
  binary_parser.cc
)
target_link_libraries(binary_parser PRIVATE Boost::program_options rlclientlib)

