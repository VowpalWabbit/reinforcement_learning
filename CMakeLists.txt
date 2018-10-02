cmake_minimum_required(VERSION 3.7)
project(reinforcement_learning)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
# include(GNUInstallDirs)

find_package(Boost COMPONENTS unit_test_framework system REQUIRED)
message(STATUS "Boost version: ${Boost_VERSION}")

# set(CMAKE_CXX_EXTENSIONS OFF)

# TODO RapidJson should be a submodule not checked in source
set(rapidjson_inc_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../rapidjson/include)

# TODO convert vw to a cmake project for easy target consumption
set(vw_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../vowpalwabbit)
# set(vw_libs ${vw_lib_dir}/vw ${vw_lib_dir}/allreduce)
set(vw_inc_dirs ${CMAKE_CURRENT_SOURCE_DIR}/../explore ${CMAKE_CURRENT_SOURCE_DIR}/../vowpalwabbit)

add_library(vw SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(vw PROPERTIES
  IMPORTED_LOCATION "${vw_lib_dir}/vw"
  INTERFACE_INCLUDE_DIRECTORIES "${vw_inc_dirs}"
)

foreach(vw_lib IN ITEMS ${vw_libs})
  if(NOT EXISTS "${vw_lib}")
    message( FATAL_ERROR "${vw_lib} not found. Build vw first." )
  endif()
endforeach()
message(STATUS "VW libs found: ${vw_libs}")

add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-DNDEBUG)

if(WIN32)
	# /MP multi processor
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP -D__SSE2__")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fomit-frame-pointer -fno-strict-aliasing -msse2 -mfpmath=sse")
endif()

# Global
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Target based
# set_target_properties(lib1 PROPERTIES POSITION_INDEPENDENT_CODE ON)

# TODO add FATAL_ERROR for cpprest not found
# TODO Compile cpprest into its own lib so that it doesn't have to be consumed
find_package(cpprestsdk REQUIRED)

# Link 'warnings' to include warnings
# add_library(warnings INTERFACE)

# if(MSVC)
#   target_compile_options(warnings INTERFACE /W4)
# else()
#   target_compile_options(warnings INTERFACE
#     -Wall
#     -Wextra
#     -Wnon-virtual-dtor
#     -Wold-style-cast
#     -Wunused
#     -Woverloaded-virtual
#     -Wpedantic
#     -Wduplicated-cond
#     -Wduplicated-branches
#     -Wnull-dereference
#     -Wuseless-cast
#     -Wdouble-promotion)
# endif()

#add_subdirectory(bindings)
#add_subdirectory(doc)
add_subdirectory(ext_libs)
add_subdirectory(rlclientlib)

# Add project in examples so that full build system is generated
#add_subdirectory(examples EXCLUDE_FROM_ALL)
