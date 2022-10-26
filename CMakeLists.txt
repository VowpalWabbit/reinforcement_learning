cmake_minimum_required(VERSION 3.5)
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

if(WIN32)
  # Due to needing to configure the CMAKE platform, this needs to be included before the
  # top-level project() declaration.
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms/win32.cmake)
  message(STATUS "WinSDK Version: ${CMAKE_SYSTEM_VERSION}")
endif()

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT GENERATOR_IS_MULTI_CONFIG)
  message(STATUS "No build type selected, defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Set this to on so that tooling can make use of the outputted compile commands (such as clang-tidy)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

project(reinforcement_learning)

#set(CMAKE_CXX_STANDARD 11)
# Add support for building library with latest version of C++ supported by your compiler
# Copied from VW
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/")
option(USE_LATEST_STD "Override using C++11 with the latest standard the compiler offers. Default is C++11. " OFF)
include(DetectCXXStandard)
set(RL_CXX_STANDARD 11)
if(USE_LATEST_STD)
  DetectCXXStandard(RL_CXX_STANDARD)
endif()
message(STATUS "Using C++ standard: " ${RL_CXX_STANDARD})
set(CMAKE_CXX_STANDARD ${RL_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RL_BUILD_PYTHON "Build the Python bindings" OFF)
option(RL_USE_ZSTD "Whether to enable usage of zstandard compression" ON)
option(RL_STATIC_DEPS "Only use static dependencies" OFF)
option(RL_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(vw_USE_AZURE_FACTORIES "Whether to compile with the azure factories components" ON)
option(RL_OPENSSL_SYS_DEP "Use system-wide openssl library" ON)
option(RL_CPPRESTSDK_SYS_DEP "Use system-wide cpprestsdk library" ON)
option(RL_BUILD_NUGET "Build the Nuget package" OFF)
option(RL_BUILD_EXTERNAL_PARSER "Build external parser version of VW" OFF)
option(RL_USE_ASAN "Compile with AddressSanitizer" OFF)
option(RL_USE_UBSAN "Compile with UndefinedBehaviorSanitizer" OFF)

if(RL_USE_ASAN)
  add_compile_definitions(RL_USE_ASAN VW_USE_ASAN)
  if(MSVC)
    add_compile_options(/fsanitize=address /GS- /wd5072)
    add_link_options(/InferASanLibs /incremental:no /debug)
    # Workaround for MSVC ASan issue here: https://developercommunity.visualstudio.com/t/VS2022---Address-sanitizer-on-x86-Debug-/10116361
    add_compile_definitions(_DISABLE_STRING_ANNOTATION)
  else()
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g3)
    add_link_options(-fsanitize=address -fno-omit-frame-pointer -g3)
  endif()
endif()

if(RL_USE_UBSAN)
  add_compile_definitions(RL_USE_UBSAN VW_USE_UBSAN)
  if(MSVC)
    message(FATAL_ERROR "UBSan not supported on MSVC")
  else()
    add_compile_options(-fsanitize=undefined -fno-sanitize-recover -fno-omit-frame-pointer -g3)
    add_link_options(-fsanitize=undefined -fno-sanitize-recover -fno-omit-frame-pointer -g3)
  endif()
endif()

# For Nuget package don't use system libraries, use vendored ones in ext_libs instead
if(RL_BUILD_NUGET)
  set(RAPIDJSON_SYS_DEP OFF CACHE BOOL "" FORCE)
  set(FMT_SYS_DEP OFF CACHE BOOL "" FORCE)
  set(SPDLOG_SYS_DEP OFF CACHE BOOL "" FORCE)
  set(VW_ZLIB_SYS_DEP OFF CACHE BOOL "" FORCE)
  set(VW_BOOST_MATH_SYS_DEP OFF CACHE BOOL "" FORCE)
  set(RL_OPENSSL_SYS_DEP OFF CACHE BOOL "" FORCE)
  set(RL_CPPRESTSDK_SYS_DEP OFF CACHE BOOL "" FORCE)
  set(RL_STATIC_DEPS ON)
endif()

if(RL_STATIC_DEPS)
  if(WIN32)
    set(STATIC_LIB_SUFFIXES ".lib" ".a")
  else()
    set(STATIC_LIB_SUFFIXES ".a")
  endif()

  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
  set(STATIC_LINK_VW_JAVA ON CACHE BOOL "")
  set(Boost_USE_STATIC_LIBS ON CACHE BOOL "")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${STATIC_LIB_SUFFIXES})
endif()

# We also require Boost::uuid, but that is a header-only dependency and is unable to be found
# using find_package as a component, which means it is possible to have a successfully configured
# build that will nonetheless not work due to missing dependencies.
find_package(Boost COMPONENTS unit_test_framework system program_options thread REQUIRED)

# On MacOS, Boost::thread needs to be linked separately.
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  find_package(Boost COMPONENTS thread REQUIRED)
endif()

include(ProcessorCount)
ProcessorCount(NumProcessors)
message("Number of processors: ${NumProcessors}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/nprocs.txt ${NumProcessors})

# This provides the variables such as CMAKE_INSTALL_LIBDIR for installation paths.
include(GNUInstallDirs)

include(ext_libs/ext_libs.cmake)

add_subdirectory(rlclientlib)
add_subdirectory(rlclientlib/extensions)
add_subdirectory(examples)
add_subdirectory(test_tools/joiner)
add_subdirectory(test_tools/sender_test)
add_subdirectory(test_tools/example_gen)

if(RL_BUILD_EXTERNAL_PARSER)
  add_subdirectory(external_parser)
endif()

# enable_testing should be run after ext_libs so that the vw unit tests arent turned on.
enable_testing()

# Since bindings will add tests, if appropriate, it needs to be included after enable_testing().
add_subdirectory(bindings)

add_subdirectory(unit_test)
add_subdirectory(unit_test/extensions)
if(RL_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

# Add the nuget subdirectory last
if(RL_BUILD_NUGET)
  if(WIN32)
    add_subdirectory(nuget)
  else()
    message(FATAL_ERROR "Nuget package must be built on Windows")
  endif()
endif()
