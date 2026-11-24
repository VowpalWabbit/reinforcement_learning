set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(FindDotnet)

set(RL_DOTNET_TARGET_FRAMEWORK "net10.0" CACHE STRING "Framework used to build and run the managed CLI tests")
set_property(CACHE RL_DOTNET_TARGET_FRAMEWORK PROPERTY STRINGS netcoreapp3.1 net10.0)
if(NOT RL_DOTNET_TARGET_FRAMEWORK STREQUAL "netcoreapp3.1" AND NOT RL_DOTNET_TARGET_FRAMEWORK STREQUAL "net10.0")
  message(FATAL_ERROR "RL_DOTNET_TARGET_FRAMEWORK must be netcoreapp3.1 or net10.0")
endif()
set(RL_DOTNET_OUTPUT_DIRECTORY "$<TARGET_FILE_DIR:rlnetnative>/${RL_DOTNET_TARGET_FRAMEWORK}")

# note: this change was made since building with Ninja does not add suffixes
#       but, using the VS generator does. rl.net uses dllimport to load rlnetnative.
#       This is a workaround to make sure the correct dll is used.
if (WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
  set(CMAKE_DEBUG_POSTFIX "")
endif()

add_subdirectory(rl.net.native)
add_subdirectory(rl.net)
add_subdirectory(rl.net.cli)
add_subdirectory(rl.net.cli.test)
