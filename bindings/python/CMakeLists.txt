if(BUILD_PYTHON)
  if(NOT DEFINED PY_VERSION)
    message(SEND_ERROR "PY_VERSION must be defined.")
  endif()

  find_package(SWIG REQUIRED)
  find_package(PythonInterp ${PY_VERSION} REQUIRED)

  set(pip_folder ${CMAKE_CURRENT_SOURCE_DIR}/rl_client/)
  add_custom_target(_rl_client ALL)
  add_custom_command(TARGET _rl_client
    COMMAND ${SWIG_EXECUTABLE} -c++ -python -o rl_client_wrap.cc rl_client.i
    COMMAND ${CMAKE_COMMAND} -E copy rl_client.py ${pip_folder}
    COMMAND ${PYTHON_EXECUTABLE} setup.py bdist_wheel
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rl_client.i ${CMAKE_CURRENT_SOURCE_DIR}/py_api.h ${CMAKE_CURRENT_SOURCE_DIR}/py_api.cc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_dependencies(_rl_client rlclientlib vw allreduce)
endif()
