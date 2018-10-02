cmake_minimum_required(VERSION 3.7)
project(reinforcement_learning)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
# include(GNUInstallDirs)

include(ProcessorCount)
ProcessorCount(N)
message(STATUS "ProcessorCount: ${N}")
message(STATUS "Invoke make as \"make -j ${N}\"")

# TODO Compile cpprest into its own lib so that it doesn't have to be consumed
find_package(cpprestsdk REQUIRED)
find_package(Boost COMPONENTS unit_test_framework system program_options REQUIRED)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../rapidjson/include)

set(vw_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../vowpalwabbit)
set(vw_inc_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../explore ${CMAKE_CURRENT_SOURCE_DIR}/../vowpalwabbit)

add_library(vw STATIC IMPORTED)
set_target_properties(vw PROPERTIES
  IMPORTED_LOCATION "${vw_lib_dir}/libvw.a"
  INTERFACE_INCLUDE_DIRECTORIES "${vw_inc_dirs}"
)

add_library(allreduce STATIC IMPORTED)
set_target_properties(allreduce PROPERTIES
  IMPORTED_LOCATION "${vw_lib_dir}/liballreduce.a"
  INTERFACE_INCLUDE_DIRECTORIES "${vw_inc_dirs}"
)

enable_testing()

add_subdirectory(bindings)
add_subdirectory(doc)
add_subdirectory(ext_libs)
add_subdirectory(rlclientlib)

add_subdirectory(examples EXCLUDE_FROM_ALL)
