include(FindPackageHandleStandardArgs)

# On Linux this should find the .so and on Windows this should find the .lib import for the dll
find_library(ONNXRUNTIME_LIB
  NAMES onnxruntime
  PATHS "${ONNXRUNTIME_ROOT}"
  PATH_SUFFIXES lib
)

set(ONNXRUNTIME_REQUIRED_IMPLIB_VAR_NAME "")
# NO_DEFAULT_PATH is used because Windows ships an onnxruntime.dll in System32
# and we do not want to find that one, so we ONLY search ONNXRUNTIME_ROOT
if(WIN32)
  find_file(ONNXRUNTIME_DLL
    NAMES onnxruntime.dll
    PATHS "${ONNXRUNTIME_ROOT}"
    PATH_SUFFIXES bin
    NO_DEFAULT_PATH
  )
  set(ONNXRUNTIME_IMP_LIB "${ONNXRUNTIME_LIB}")
  set(ONNXRUNTIME_LIB "${ONNXRUNTIME_DLL}")
  set(ONNXRUNTIME_REQUIRED_IMPLIB_VAR_NAME "ONNXRUNTIME_IMP_LIB")
endif()

find_path(ONNXRUNTIME_INCLUDE_DIR_ROOT
  NAMES include/onnxruntime/core/session
  PATHS "${ONNXRUNTIME_ROOT}"
  PATH_SUFFIXES include
)

# There may be many files names version.h in the search path. So we find the one
# with the expected preprocessor definition ONNXRUNTIME_VERSION_STRING and then
# read the version file from that one.
file(GLOB_RECURSE FOUND_VERSION_FILES "${ONNXRUNTIME_INCLUDE_DIR_ROOT}/**/version.h")
foreach(VERSION_FILE ${FOUND_VERSION_FILES})
  file(READ ${VERSION_FILE} VERSION_FILE_CONTENTS)
  string(FIND "${VERSION_FILE_CONTENTS}" "ONNXRUNTIME_VERSION_STRING" FIND_RESULT)

  if(NOT ${FIND_RESULT} EQUAL -1)
    string(REGEX MATCH "\"(.*)\"" ONNX_VERSION_OUTPUT_UNUSED ${VERSION_FILE_CONTENTS})
    set(ONNXRUNTIME_VERSION ${CMAKE_MATCH_1})
    break()
  endif ()
endforeach()

# The ${ONNXRUNTIME_REQUIRED_IMPLIB_VAR_NAME} is intentionally different to the
# others as the value of this variable is the name of the implib var IF it is
# required and blank otherwise.
find_package_handle_standard_args(OnnxRuntime
  REQUIRED_VARS ONNXRUNTIME_LIB ONNXRUNTIME_INCLUDE_DIR_ROOT ${ONNXRUNTIME_REQUIRED_IMPLIB_VAR_NAME}
  VERSION_VAR ONNXRUNTIME_VERSION)

if(ONNXRUNTIME_FOUND)
  add_library(onnxruntime SHARED IMPORTED)
  set_target_properties(onnxruntime PROPERTIES
    IMPORTED_IMPLIB "${ONNXRUNTIME_IMP_LIB}" # On non-Windows this is empty and therefore ignored.
    IMPORTED_LOCATION "${ONNXRUNTIME_LIB}"
  )

  set(ONNXRUNTIME_INCLUDE_DIRS "${ONNXRUNTIME_INCLUDE_DIR_ROOT}/include/onnxruntime")
  target_include_directories(onnxruntime INTERFACE ${ONNXRUNTIME_INCLUDE_DIRS})
endif()

mark_as_advanced(ONNXRUNTIME_ROOT)