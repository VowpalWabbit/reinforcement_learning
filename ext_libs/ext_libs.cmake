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
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vowpal_wabbit)
  if(USE_ZSTD)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/zstd/build/cmake)
  endif()
else()
  set(VW_INSTALL OFF CACHE BOOL "")
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vowpal_wabbit EXCLUDE_FROM_ALL)
  if(USE_ZSTD)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/zstd/build/cmake EXCLUDE_FROM_ALL)
  endif()
endif()