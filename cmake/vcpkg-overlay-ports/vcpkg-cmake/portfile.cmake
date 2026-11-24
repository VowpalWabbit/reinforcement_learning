set(helper_source "${CURRENT_BUILDTREES_DIR}/patched-helper")
set(pinned_helper "${CMAKE_CURRENT_LIST_DIR}/../../../ext_libs/vcpkg/ports/vcpkg-cmake")
file(READ "${pinned_helper}/vcpkg-port-config.cmake" helper_contents)
string(REPLACE "\r\n" "\n" helper_contents "${helper_contents}")
string(SHA256 helper_hash "${helper_contents}")
if(NOT helper_hash STREQUAL "f0a30f77c8f5e3ac40436fe2518a61ad067f2955c7ef3be6d6a0ca4b81cd2a45")
    message(FATAL_ERROR "The pinned vcpkg-cmake helper changed; review the native pkgconf overlay.")
endif()
file(REMOVE_RECURSE "${helper_source}")
file(COPY "${pinned_helper}/" DESTINATION "${helper_source}")
set(pinned_scripts "${pinned_helper}/../../scripts/cmake")
file(READ "${pinned_scripts}/vcpkg_acquire_msys.cmake" msys_contents)
string(REPLACE "\r\n" "\n" msys_contents "${msys_contents}")
string(SHA256 msys_hash "${msys_contents}")
if(NOT msys_hash STREQUAL "251a30badbb2082236811ba6975680d0d012a20dd55db7e39ccbc6cc49d67287")
    message(FATAL_ERROR "The pinned MSYS acquisition helper changed; review the native pkgconf overlay.")
endif()
# Native MinGW packages do not require the POSIX runtime.
string(REPLACE [[REGEX "^mingw64"]] [[REGEX "^(mingw-|mingw64)"]] msys_contents "${msys_contents}")
file(WRITE "${helper_source}/vcpkg_acquire_msys.cmake" "${msys_contents}")
file(APPEND "${helper_source}/vcpkg-port-config.cmake" [=[

if(VCPKG_HOST_IS_WINDOWS)
    include("${CMAKE_CURRENT_LIST_DIR}/vcpkg_acquire_msys.cmake")
endif()
]=])
include("${helper_source}/portfile.cmake")
file(INSTALL
    "${helper_source}/vcpkg_acquire_msys.cmake"
    "${pinned_scripts}/compile_wrapper_consider_clang-cl.patch"
    DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
