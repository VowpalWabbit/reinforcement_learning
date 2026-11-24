# Check that variables are set
if(NOT DEFINED RL_NUGET_PACKAGE_NAME OR NOT DEFINED RL_NUGET_PACKAGE_VERSION OR NOT DEFINED NATIVE_NUGET_PLATFORM_TAG)
  message(FATAL_ERROR "When building Nuget package, must define variables: RL_NUGET_PACKAGE_NAME, RL_NUGET_PACKAGE_VERSION, NATIVE_NUGET_PLATFORM_TAG")
endif()

# Generate NuGet package specification file from template
configure_file(rlclientlib.nuspec.in rlclientlib.nuspec @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rlclientlib.nuspec DESTINATION ./)

# Generate the .targets file from template
configure_file(rlclientlib.targets.in rlclientlib.targets @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rlclientlib.targets DESTINATION ./ RENAME ${RL_NUGET_PACKAGE_NAME}-v${MSVC_TOOLSET_VERSION}-${CMAKE_BUILD_TYPE}-${NATIVE_NUGET_PLATFORM_TAG}.targets)

# Build package
install(SCRIPT CreateNugetPackage.cmake)