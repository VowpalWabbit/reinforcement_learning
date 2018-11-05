add_executable(basic_usage_cpp.out
  basic_usage_cpp.cc
)

target_link_libraries(basic_usage_cpp.out PRIVATE rlclientlib)
