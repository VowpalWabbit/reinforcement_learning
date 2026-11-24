set(helper_source "${CURRENT_BUILDTREES_DIR}/patched-helper")
# Keep the helper paired with the repository's pinned registry, not a caller's vcpkg installation.
set(pinned_helper "${CMAKE_CURRENT_LIST_DIR}/../../../ext_libs/vcpkg/ports/vcpkg-boost")
file(READ "${pinned_helper}/boost-install.cmake" helper_contents)
string(REPLACE "\r\n" "\n" helper_contents "${helper_contents}")
string(SHA256 helper_hash "${helper_contents}")
if(NOT helper_hash STREQUAL "19f0f619f254311a6b55bee022a584ae1ff040d3bdb0728e646833e62c9e6dc9")
    message(FATAL_ERROR "The pinned vcpkg-boost helper changed; review the Windows rename overlay.")
endif()
file(REMOVE_RECURSE "${helper_source}")
file(COPY "${pinned_helper}/" DESTINATION "${helper_source}")
vcpkg_apply_patches(
    SOURCE_PATH "${helper_source}"
    PATCHES "${CMAKE_CURRENT_LIST_DIR}/windows-directory-rename.patch")
include("${helper_source}/portfile.cmake")
