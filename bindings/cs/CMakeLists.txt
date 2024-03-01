set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(FindDotnet)

add_subdirectory(rl.net.native)
add_subdirectory(rl.net)
add_subdirectory(rl.net.cli)
add_subdirectory(rl.net.flatbuffer)
add_subdirectory(rl.net.cli.test)
