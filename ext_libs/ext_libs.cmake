add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/fakeit)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/date)

if(RL_BUILD_PYTHON)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/pybind11)
endif()

# Nuget build relies on installing libraries with cmake --install
# This means that subdirectories cannot be EXCLUDE_FROM_ALL
if(RL_BUILD_NUGET)
  set(VW_INSTALL ON CACHE BOOL "" FORCE)
  set(CPPREST_INSTALL ON CACHE BOOL "" FORCE)
  set(OPENSSL_INSTALL ON)
  set(RL_ext_libs_exclude_from_all "")
else()
  set(VW_INSTALL OFF CACHE BOOL "")
  set(OPENSSL_INSTALL OFF)
  set(RL_ext_libs_exclude_from_all EXCLUDE_FROM_ALL)
endif()

# Build VW submodule
set(BUILD_TESTING OFF CACHE BOOL "")
set(BUILD_JAVA OFF CACHE BOOL "")
set(BUILD_PYTHON OFF CACHE BOOL "")
set(BUILD_DOCS OFF CACHE BOOL "")
set(WARNINGS OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/vowpal_wabbit ${RL_ext_libs_exclude_from_all})

# Build ZStandard
if(RL_USE_ZSTD)
  set(ZSTD_BUILD_STATIC ON CACHE BOOL "")
  set(ZSTD_BUILD_SHARED OFF CACHE BOOL "")
  set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "")
  set(ZSTD_BUILD_TESTS OFF CACHE BOOL "")
  # Dynamic link to MSVC runtime libraries
  set(ZSTD_USE_STATIC_RUNTIME OFF CACHE BOOL "")
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/zstd/build/cmake ${RL_ext_libs_exclude_from_all})
endif()

# Build OpenSSL and Cpprestsdk
if(vw_USE_AZURE_FACTORIES)
  if(RL_OPENSSL_SYS_DEP)
    # Link to system OpenSSL library
    find_package(OpenSSL REQUIRED)
  else()
    # Build vendored OpenSSL library
    # OpenSSL uses its own Perl script build system, so in CMake it needs to be an ExternalProject
    include(ExternalProject)
    include(FindPerl)
    if(WIN32)
      find_program(MAKE_EXECUTABLE NAMES "nmake" REQUIRED)
      find_program(NASM_EXECUTABLE NAMES "nasm" REQUIRED)
      set(STATIC_LIB_SUFFIX ".lib")
    else()
      find_program(MAKE_EXECUTABLE NAMES "make" "gmake" REQUIRED)
      set(STATIC_LIB_SUFFIX ".a")
    endif()

    set(RL_OPENSSL_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/openssl")
    set(RL_OPENSSL_BUILD_PREFIX_DIR "${CMAKE_CURRENT_BINARY_DIR}/openssl_external")
    set(RL_OPENSSL_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/openssl_external/build")
    set(RL_OPENSSL_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/openssl_external/install")
    if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
      set(RL_OPENSSL_DEBUG_OR_RELEASE "--debug")
    else()
      set(RL_OPENSSL_DEBUG_OR_RELEASE "--release")
    endif()

    ExternalProject_Add(
      openssl_external
      PREFIX ${RL_OPENSSL_BUILD_PREFIX_DIR}
      SOURCE_DIR ${RL_OPENSSL_SOURCE_DIR}
      BINARY_DIR ${RL_OPENSSL_BINARY_DIR}
      INSTALL_DIR ${RL_OPENSSL_INSTALL_DIR}
      DOWNLOAD_COMMAND "" # files should alread exist in git submodule
      CONFIGURE_COMMAND "" # use custom perl_configure step instead
      BUILD_COMMAND "${MAKE_EXECUTABLE}"
      INSTALL_COMMAND "${MAKE_EXECUTABLE}" install_sw
      BUILD_BYPRODUCTS "${RL_OPENSSL_INSTALL_DIR}/lib/libcrypto${STATIC_LIB_SUFFIX}" "${RL_OPENSSL_INSTALL_DIR}/lib/libssl${STATIC_LIB_SUFFIX}"
      USES_TERMINAL_BUILD TRUE
      USES_TERMINAL_INSTALL TRUE
    )
    ExternalProject_Add_Step(
      openssl_external perl_configure
      COMMENT "Running perl_configure step in OpenSSL external project..."
      COMMAND "${PERL_EXECUTABLE}" "${RL_OPENSSL_SOURCE_DIR}/Configure" --prefix=${RL_OPENSSL_INSTALL_DIR} --openssldir=${RL_OPENSSL_INSTALL_DIR}/ssl ${RL_OPENSSL_DEBUG_OR_RELEASE} no-shared no-tests
      WORKING_DIRECTORY ${RL_OPENSSL_BINARY_DIR}
      DEPENDERS build
    )
    add_library(OpenSSL::Crypto STATIC IMPORTED)
    add_dependencies(OpenSSL::Crypto openssl_external)
    set_target_properties(
      OpenSSL::Crypto PROPERTIES
      IMPORTED_LOCATION "${RL_OPENSSL_INSTALL_DIR}/lib/libcrypto${STATIC_LIB_SUFFIX}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      INTERFACE_INCLUDE_DIRECTORIES "${RL_OPENSSL_INSTALL_DIR}/include"
    )
    add_library(OpenSSL::SSL STATIC IMPORTED)
    add_dependencies(OpenSSL::SSL openssl_external)
    set_target_properties(
      OpenSSL::SSL PROPERTIES
      IMPORTED_LOCATION "${RL_OPENSSL_INSTALL_DIR}/lib/libssl${STATIC_LIB_SUFFIX}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      INTERFACE_INCLUDE_DIRECTORIES "${RL_OPENSSL_INSTALL_DIR}/include"
      INTERFACE_LINK_LIBRARIES OpenSSL::Crypto
    )

    if(OPENSSL_INSTALL)
      if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
        install(FILES "${RL_OPENSSL_INSTALL_DIR}/lib/libcrypto${STATIC_LIB_SUFFIX}" DESTINATION ${CMAKE_INSTALL_LIBDIR} RENAME "libcryptod${STATIC_LIB_SUFFIX}")
        install(FILES "${RL_OPENSSL_INSTALL_DIR}/lib/libssl${STATIC_LIB_SUFFIX}" DESTINATION ${CMAKE_INSTALL_LIBDIR} RENAME "libssld${STATIC_LIB_SUFFIX}")
      else()
        install(FILES "${RL_OPENSSL_INSTALL_DIR}/lib/libcrypto${STATIC_LIB_SUFFIX}" DESTINATION ${CMAKE_INSTALL_LIBDIR})
        install(FILES "${RL_OPENSSL_INSTALL_DIR}/lib/libssl${STATIC_LIB_SUFFIX}" DESTINATION ${CMAKE_INSTALL_LIBDIR})
      endif()
    endif()
  endif() # RL_OPENSSL_SYS_DEP

  if(RL_CPPRESTSDK_SYS_DEP)
    # Link to system cpprestsdk library
    find_package(cpprestsdk REQUIRED)
  else()
    # Build vendored cpprestsdk library
    # Get cpprestsdk to use the zlib static build inside of VW (target zlibstatic)
    if(NOT VW_ZLIB_SYS_DEP)
      add_library(cpprestsdk_zlib_internal INTERFACE)
      target_link_libraries(cpprestsdk_zlib_internal INTERFACE zlibstatic)
    endif()

    # Get cpprestsdk to use vendored openssl inside of rlclientlib
    if(NOT RL_OPENSSL_SYS_DEP)
      add_library(cpprestsdk_openssl_internal INTERFACE)
      target_link_libraries(cpprestsdk_openssl_internal INTERFACE OpenSSL::SSL)
    endif()

    set(WERROR OFF CACHE BOOL "")
    set(CPPREST_ABI_TAG "" CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/cpprestsdk ${RL_ext_libs_exclude_from_all})
    add_library(cpprestsdk::cpprest ALIAS cpprest)
  endif() # RL_CPPREST_SYS_DEP
endif() # vw_USE_AZURE_FACTORIES