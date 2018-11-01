cmake_minimum_required(VERSION 3.5)
project(reinforcement_learning)

# TODO Compile cpprest into its own separate lib so that it doesn't have to be consumed
find_package(cpprestsdk REQUIRED)
find_package(Boost COMPONENTS unit_test_framework system REQUIRED)

set(CMAKE_CXX_STANDARD 11)

include(ProcessorCount)
ProcessorCount(NumProcessors)
message("Number of processors: ${NumProcessors}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nprocs.txt ${NumProcessors})

add_subdirectory(bindings EXCLUDE_FROM_ALL)
add_subdirectory(doc EXCLUDE_FROM_ALL)
add_subdirectory(ext_libs)
add_subdirectory(rlclientlib)
add_subdirectory(examples)

# enable_testing should be run after ext_libs so that the vw unit tests arent turned on.
enable_testing()
add_subdirectory(unit_test)
