set(pinned_registry "${CMAKE_CURRENT_LIST_DIR}/../../../ext_libs/vcpkg")
set(pinned_port_tree "ad5a49006f73b45b715299515f31164131b51982")
set(port_source "${CURRENT_BUILDTREES_DIR}/patched-port")
set(port_archive "${CURRENT_BUILDTREES_DIR}/pinned-port.tar")
vcpkg_find_acquire_program(GIT)
execute_process(
    COMMAND "${GIT}" -C "${pinned_registry}" rev-parse HEAD
    OUTPUT_VARIABLE registry_commit
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)
if(NOT registry_commit STREQUAL "d5ec528843d29e3a52d745a64b469f810b2cedbf")
    message(FATAL_ERROR "The pinned vcpkg registry changed; review the macOS zlib overlay.")
endif()
vcpkg_execute_required_process(
    COMMAND "${GIT}" -C "${pinned_registry}" archive --format=tar
        "--output=${port_archive}" "${pinned_port_tree}"
    WORKING_DIRECTORY "${CURRENT_BUILDTREES_DIR}"
    LOGNAME restore-pinned-port)
file(REMOVE_RECURSE "${port_source}")
file(ARCHIVE_EXTRACT INPUT "${port_archive}" DESTINATION "${port_source}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/modern-macos.patch" DESTINATION "${port_source}")
vcpkg_apply_patches(
    SOURCE_PATH "${port_source}"
    PATCHES "${CMAKE_CURRENT_LIST_DIR}/add-macos-patch.patch")
include("${port_source}/portfile.cmake")
