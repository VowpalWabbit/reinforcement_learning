add_executable(joiner.out
  main.cc
  text_converter.cc
)

target_link_libraries(joiner.out PRIVATE Boost::program_options rlclientlib)
