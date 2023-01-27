add_library(rl_swig_interface STATIC lib.h lib.cc)
target_link_libraries(rl_swig_interface PRIVATE rlclientlib)

set(JAVA_AWT_LIBRARY NotNeeded)
set(JAVA_JVM_LIBRARY NotNeeded)
set(JAVA_INCLUDE_PATH2 NotNeeded)
set(JAVA_AWT_INCLUDE_PATH NotNeeded)
find_package(JNI REQUIRED)

# set policy CMP0078
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set_property(SOURCE java.i PROPERTY CPLUSPLUS ON)
swig_add_library(rl_java_swig
    TYPE SHARED
    LANGUAGE java
    SOURCES java.i
)

target_link_libraries(rl_java_swig PRIVATE JNI::JNI rl_swig_interface)
target_include_directories(rl_java_swig PRIVATE ${CMAKE_CURRENT_LIST_DIR})