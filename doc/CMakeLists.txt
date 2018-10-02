find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
  message(FATAL_ERROR "Doxygen needs to be installed to generate the doxygen documentation")
endif()

set(DOXYGEN_SKIP_DOT TRUE)

add_custom_target(copy_docs_file_build_dir
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/cpp ${CMAKE_CURRENT_BINARY_DIR}/cpp
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/python ${CMAKE_CURRENT_BINARY_DIR}/python
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/doxygen.css ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/layout.xml ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(doxygen_cpp
  COMMAND ${DOXYGEN_EXECUTABLE}
  WORKING_DIRECTORY cpp
)

add_custom_target(doxygen_python
  COMMAND ${DOXYGEN_EXECUTABLE}
  WORKING_DIRECTORY python
)

add_dependencies(doxygen_cpp copy_docs_file_build_dir)
add_dependencies(doxygen_python copy_docs_file_build_dir)

add_custom_target(doc DEPENDS doxygen_cpp doxygen_python)
