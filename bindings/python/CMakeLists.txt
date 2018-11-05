if(NOT DEFINED RL_PY_VERSION)
  set(RL_PY_VERSION 2.7)
endif()

message("Building Python module for Python version ${RL_PY_VERSION}")
find_package(PythonLibs ${RL_PY_VERSION})
find_package(SWIG)

if(SWIG_FOUND AND PythonLibs_FOUND)
include(${SWIG_USE_FILE})
  if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND NOT WIN32)
    SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-dynamic")
  endif()

  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(CMAKE_STATIC_LIBRARY_PREFIX "")
  set(CMAKE_SWIG_FLAGS "-c++")

  include_directories(${PYTHON_INCLUDE_PATH})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  set_source_files_properties(rl_client.i PROPERTIES CPLUSPLUS ON)
  swig_add_module(rl_client python rl_client.i py_api.cc)
  swig_link_libraries(rl_client rlclientlib ${PYTHON_LIBRARIES})

  add_custom_command(TARGET ${SWIG_MODULE_rl_client_REAL_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/rl_client.py ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()
