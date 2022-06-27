# Check that variables are set
if(NOT DEFINED RL_NUGET_PACKAGE_NAME OR NOT DEFINED RL_NUGET_PACKAGE_VERSION OR NOT DEFINED NATIVE_NUGET_PLATFORM_TAG)
  message(FATAL_ERROR "When building Nuget package, must define variables: RL_NUGET_PACKAGE_NAME, RL_NUGET_PACKAGE_VERSION, NATIVE_NUGET_PLATFORM_TAG")
endif()

# Generate NuGet package specification file from template
configure_file(rlclientlib.nuspec.in rlclientlib.nuspec @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rlclientlib.nuspec DESTINATION ./)

# Copy the .targets file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/rlclientlib.targets DESTINATION ./ RENAME ${RL_NUGET_PACKAGE_NAME}-v${MSVC_TOOLSET_VERSION}-${NATIVE_NUGET_PLATFORM_TAG}.targets)

# Build package
install(CODE "message(\"Building nuget package with command: execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/nuget.exe pack rlclientlib.nuspec WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX})\")")
install(CODE "execute_process(COMMAND \"${CMAKE_CURRENT_SOURCE_DIR}/nuget.exe\" pack rlclientlib.nuspec WORKING_DIRECTORY \"${CMAKE_INSTALL_PREFIX}\")")