find_library(ONNXRUNTIME_LIB 
  NAMES onnxruntime
  )
find_path(ONNXRUNTIME_INCLUDE_DIR_ROOT NAMES include/onnxruntime/core/session)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(onnxruntime
  DEFAULT_MSG ONNXRUNTIME_LIB ONNXRUNTIME_INCLUDE_DIR_ROOT)

if(ONNXRUNTIME_FOUND)
  add_library(onnxruntime SHARED IMPORTED)
  set_target_properties(onnxruntime PROPERTIES IMPORTED_LOCATION ${ONNXRUNTIME_LIB})
  
  set(ONNXRUNTIME_INCLUDE_DIRS ${ONNXRUNTIME_INCLUDE_DIR_ROOT}/include/onnxruntime)
  
  target_include_directories(onnxruntime INTERFACE ${ONNXRUNTIME_INCLUDE_DIRS})
else()
  set(ONNXRUNTIME_INCLUDE_DIR_ROOT)
endif()