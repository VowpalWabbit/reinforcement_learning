# macOS vcpkg compatibility overlay

Root and standalone parser CMake builds enable this overlay on macOS hosts before
`project()`, using the already available `CMAKE_HOST_APPLE` variable.

The zlib overlay retains version 1.2.13, its source SHA512, dependencies, packaging,
and all four patches from vcpkg port tree
`ad5a49006f73b45b715299515f31164131b51982`. It restores that exact tree from the
checked-in vcpkg Git history and adds one source patch: modern Apple platforms
must not use the classic Mac OS `fdopen` replacement, which breaks the macOS 15.5
SDK's stdio declaration. Classic Mac OS behavior is retained.

The overlay rejects a changed vcpkg checkout. When updating the pinned registry or
zlib version, review whether this patch is still necessary and update or remove
the overlay explicitly. A shallow vcpkg checkout must first be unshallowed, as
the shared CI setup does. Windows and Linux builds do not enable this overlay.
