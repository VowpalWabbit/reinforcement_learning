add_executable(override_interface.out
  override_interface.cc
)

target_link_libraries(override_interface.out PRIVATE rlclientlib)
