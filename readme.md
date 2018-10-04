
# Dependencies
Note: this needs to be tested
```
# Windows
vcpkg install rapidjson:x64-windows
vcpkg install cpprestsdk:x64-windows
vcpkg install zlib:x64-windows
vcpkg install boost-system:x64-windows
vcpkg install boost-program-options:x64-windows
vcpkg install boost-test:x64-windows

# Linux
vcpkg install rapidjson:x64-linux
vcpkg install cpprestsdk:x64-linux
vcpkg install zlib:x64-linux
vcpkg install boost-system:x64-linux
vcpkg install boost-program-options:x64-linux
vcpkg install boost-test:x64-linux
```

# Build
```
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake
make -j
```

## Make targets
- `doc` - Python and C++ docs
- `_rl_client` - Python bindings
- `rlclient` - rlclient library
- `rltest` - unit tests
