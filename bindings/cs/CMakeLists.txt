set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(FindDotnet)

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
