cmake_minimum_required(VERSION 3.5)

if(WIN32)
  # Due to needing to configure the CMAKE platform, this needs to be included before the
  # top-level project() declaration.
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms/win32.cmake)
  message(STATUS "WinSDK Version: ${CMAKE_SYSTEM_VERSION}")
endif()

project(reinforcement_learning)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_PYTHON "Build the Python bindings" OFF)
option(USE_ZSTD "Whether to enable usage of zstandard compression" ON)
option(RL_STATIC_DEPS "Only use static dependencies" ON)

option(vw_USE_AZURE_FACTORIES "Whether to compile with the azure factories components" ON)

# TODO Compile cpprest into its own separate lib so that it doesn't have to be consumed
if (vw_USE_AZURE_FACTORIES)
    find_package(cpprestsdk REQUIRED)
endif()

if(RL_STATIC_DEPS)
    if(WIN32)
        set(STATIC_LIB_SUFFIXES ".lib" ".a")
    else()
        set(STATIC_LIB_SUFFIXES ".a")
    endif()

    set(BUILD_SHARED_LIBS OFF)
    set(STATIC_LINK_VW_JAVA ON CACHE BOOL "")
    set(Boost_USE_STATIC_LIBS ON)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${STATIC_LIB_SUFFIXES})
endif()

find_package(OpenSSL REQUIRED)

# We also require Boost::uuid, but that is a header-only dependency and is unable to be found
# using find_package as a component, which means it is possible to have a successfully configured
# build that will nonetheless not work due to missing dependencies.
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

include(ext_libs/ext_libs.cmake)

add_subdirectory(doc EXCLUDE_FROM_ALL)
add_subdirectory(rlclientlib)
add_subdirectory(rlclientlib/extensions)
add_subdirectory(examples)
add_subdirectory(test_tools/joiner)
add_subdirectory(test_tools/sender_test)
add_subdirectory(test_tools/example_gen)

# enable_testing should be run after ext_libs so that the vw unit tests arent turned on.
enable_testing()

# Since bindings will add tests, if appropriate, it needs to be included after enable_testing().
add_subdirectory(bindings)

add_subdirectory(unit_test)
add_subdirectory(unit_test/extensions)
