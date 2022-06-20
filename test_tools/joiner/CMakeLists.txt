add_executable(joiner.out
  main.cc
  text_converter.cc
)

target_include_directories(joiner.out PRIVATE ${FLATBUFFERS_INCLUDE_DIR})
target_link_libraries(joiner.out PRIVATE Boost::program_options rlclientlib)
