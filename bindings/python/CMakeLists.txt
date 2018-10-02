find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs 3.6 REQUIRED)

include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_SWIG_FLAGS "-c++")

set_source_files_properties(rl_client.i PROPERTIES CPLUSPLUS ON)

swig_add_library(rl_client LANGUAGE python SOURCES rl_client.i py_api.cc)
swig_link_libraries(rl_client rlclientlib ${PYTHON_LIBRARIES})

find_package(PythonInterp 3.6 REQUIRED)
if(NOT PYTHONINTERP_FOUND)
  message(FATAL_ERROR "Python 3.6 not found")
endif()

add_custom_command(TARGET _rl_client POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/rl_client ${CMAKE_CURRENT_BINARY_DIR}/rl_client
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/setup.py ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${PYTHON_EXECUTABLE} setup.py bdist_wheel
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
