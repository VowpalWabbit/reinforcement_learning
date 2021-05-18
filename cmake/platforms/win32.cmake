# This file contains header definitions applicable to Win32
# CMAKE_SYSTEM_VERSION must come before the project is defined in the top level CMakeLists file
# https://stackoverflow.com/questions/45692367/how-to-set-msvc-target-platform-version-with-cmake

# This is largely a copy of the same file under ext_libs/vowpal_wabbit/cmake/platforms, but does
# not include the .NET bit, as we use the CMake custom command infrastructure to drive building
# .NET, due to lack of CMake support for .NET on non-Windows generators.

# TODO: Share this file with VW

if(WIN32)
  # VW targets Windows 10.0.16299.0 SDK
  set(CMAKE_SYSTEM_VERSION "10.0.16299.0" CACHE INTERNAL "Windows SDK version to target.")

  # TODO: Enable building .NET bindings via CMake on Linux 
  option(rlclientlib_BUILD_DOTNET "Build .NET bindings" ON)

  if (rlclientlib_BUILD_DOTNET)
    cmake_minimum_required(VERSION 3.14)
    
    # The MSBuild system does not get properly enlightened to C++/CLI projects (for chaining dependencies) when
    # set up through CMake (TODO: How to fix this?). This makes it so native dependencies of the underlying VW
    # native library do not get passed through the project reference properly. The fix is to do the same as on
    # windows and redirect all targets to the same place.
    SET(rlclientlib_win32_CMAKE_RUNTIME_OUTPUT_DIRECTORY_backup ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/binaries/")
  endif()
else()
  message(FATAL_ERROR "Loading Win32-specific configuration under a non-Win32 build.")
endif()