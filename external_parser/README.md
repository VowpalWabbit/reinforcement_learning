# Parser and reward calculator for joined binary schema v2 files

## File format

### payload types

payload types indicated by the below integers

- `MSG_TYPE_HEADER = 0x55555555`
- `MSG_TYPE_REGULAR = 0xFFFFFFFF`
- `MSG_TYPE_CHECKPOINT = 0x11111111`
- `MSG_TYPE_EOF = 0xAAAAAAAA`

 ### flatbuffer payloads
 
 for all flatbuffer payloads see `rlclientlib/schema/v2`

### The parser expects to see the below format

- 4 bytes containing magic `'V','W','F','B'`
- 4 bytes containing version (right now version is `1`)
- 4 bytes containing the payload type of the header i.e. `MSG_TYPE_HEADER`
- 4 bytes containing the payload (i.e. header) size
- `<size>` bytes containing the actual header payload

any further payloads will expected to have the form of:

- 4 bytes containing the payload type which can be any of the payload types listed above
- 4 bytes containing the size of the payload to follow
- `<size>` bytes containing the actual payload

A file can contain only one header but multiple `MSG_TYPE_CHECKPOINT` and `MSG_TYPE_REGULAR` messages. `MSG_TYPE_EOF` isn't mandatory but if it is encountered the parser will stop processing the file

`MSG_TYPE_CHECKPOINT` will be expected to be followed by the size of the payload and then a payload of type `CheckpointInfo` (see `FileFormat.fbs`)

`MSG_TYPE_REGULAR` will be expected to be followed by the size of the payload and then a payload of type `JoinedPayload` (see `FileFormat.fbs`)

**Note**: everything is expected to by 8-byte aligned so if any payload is not, padding is expected in between payloads and will be skipped


## Linux

### Build Linux

**Note**: To statically link set `-DSTATIC_LINK_BINARY_PARSER=ON` during `cmake`

from `external_parser`:

- mkdir build
- cd build
- cmake ..
- make -j $(nproc)

vw executable located at: `external_parser/build/vw_binary_parser/vowpalwabbit/vw`

### Run

`./vowpalwabbit/vw -d <file> --binary_parser [other vw args]`


## Windows

cmake build for windows

### Deps:

**Note**: to link statically then replace `x64-windows` with `x64-windows-static-md` during vcpkg installation and in the cmake `-DVCPKG_TARGET_TRIPLET`

**Note**: vcpkg doesn't play well with nugets in visual studio so if you are trying to build something else in visual studio that uses the below via nugets you might get linking errors

- `vcpkg install boost-filesystem:x64-windows`
- `vcpkg install boost-thread:x64-windows`
- `vcpkg install boost-program-options:x64-windows`
- `vcpkg install boost-test:x64-windows`
- `vcpkg install flatbuffers:x64-windows`

### Build:

from `external_parser` (replace `Release` with `Debug` if you want a debug build):

- mkdir build
- cd build
- cmake .. -DCMAKE_TOOLCHAIN_FILE=<VCPKG_INSTALLATION_ROOT>\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -G "Visual Studio 15 2017" -A x64 -DBUILD_FLATBUFFERS=OFF -DCMAKE_CONFIGURATION_TYPES="Release" -DWARNINGS=OFF -DWARNINGS=OFF -DWARNING_AS_ERROR=OFF -DDO_NOT_BUILD_VW_C_WRAPPER=OFF  -DBUILD_JAVA=OFF -DBUILD_PYTHON=OFF -DBUILD_TESTS=OFF -DBUILD_EXPERIMENTAL_BINDING=OFF
- <MSBUILD_PATH> /verbosity:normal /m /p:Configuration=Release;Platform=x64 vw_binary_parser.sln

`vw_binary_parser.sln` will be available under `build` and can be used to open the solution in visual studio

vw executable located at: `external_parser\build\vw_binary_parser\vowpalwabbit\Release\vw.exe`

### Run

`.\vw_binary_parser\vowpalwabbit\Release\vw.exe -d <file> --binary_parser [other vw args]`