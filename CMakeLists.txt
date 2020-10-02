cmake_minimum_required(VERSION 3.5)
project(reinforcement_learning)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_PYTHON "Build the Python bindings" OFF)
option(USE_ZSTD "Whether to enable usage of zstandard compression" ON)

# TODO Compile cpprest into its own separate lib so that it doesn't have to be consumed
find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS unit_test_framework system program_options thread REQUIRED)

# On MacOS, Boost::thread needs to be linked separately.
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(Boost COMPONENTS thread REQUIRED)
endif()

set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG") 
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug, Release" FORCE)

include(ProcessorCount)
ProcessorCount(NumProcessors)
message("Number of processors: ${NumProcessors}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nprocs.txt ${NumProcessors})

add_subdirectory(bindings EXCLUDE_FROM_ALL)
add_subdirectory(doc EXCLUDE_FROM_ALL)
add_subdirectory(ext_libs)
add_subdirectory(rlclientlib)
add_subdirectory(examples)
add_subdirectory(test_tools/joiner)
add_subdirectory(test_tools/sender_test)
add_subdirectory(test_tools/example_gen)

# enable_testing should be run after ext_libs so that the vw unit tests arent turned on.
enable_testing()
add_subdirectory(unit_test)
