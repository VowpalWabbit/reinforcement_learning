add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fakeit)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/date)

if(RL_BUILD_PYTHON)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pybind11)
endif()

set(BUILD_TESTING OFF CACHE BOOL "")
set(BUILD_JAVA OFF CACHE BOOL "")
set(BUILD_PYTHON OFF CACHE BOOL "")
set(BUILD_DOCS OFF CACHE BOOL "")
set(WARNINGS OFF CACHE BOOL "")

# Nuget build relies on installing libraries with cmake --install
# This means that subdirectories cannot be EXCLUDE_FROM_ALL
if(RL_BUILD_NUGET)
  set(VW_INSTALL ON CACHE BOOL "" FORCE)
  set(CPPREST_INSTALL ON CACHE BOOL "" FORCE)
  set(RL_ext_libs_exclude_from_all "")
else()
  set(VW_INSTALL OFF CACHE BOOL "")
  set(RL_ext_libs_exclude_from_all EXCLUDE_FROM_ALL)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vowpal_wabbit ${RL_ext_libs_exclude_from_all})

if(vw_USE_AZURE_FACTORIES AND (NOT RL_CPPRESTSDK_SYS_DEP))
  set(WERROR OFF CACHE BOOL "")
  set(CPPREST_ABI_TAG "" CACHE BOOL "" FORCE)
  # use the zlib static build inside of VW
  if(NOT VW_ZLIB_SYS_DEP)
    add_library(cpprestsdk_zlib_internal INTERFACE)
    target_link_libraries(cpprestsdk_zlib_internal INTERFACE zlibstatic)
  endif()
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/cpprestsdk ${RL_ext_libs_exclude_from_all})
  add_library(cpprestsdk::cpprest ALIAS cpprest)
endif()

if(USE_ZSTD)
  set(ZSTD_BUILD_STATIC ON CACHE BOOL "")
  set(ZSTD_BUILD_SHARED OFF CACHE BOOL "")
  set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "")
  set(ZSTD_BUILD_TESTS OFF CACHE BOOL "")
  # Dynamic link to MSVC runtime libraries
  set(ZSTD_USE_STATIC_RUNTIME OFF CACHE BOOL "")
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/zstd/build/cmake ${RL_ext_libs_exclude_from_all})
endif()