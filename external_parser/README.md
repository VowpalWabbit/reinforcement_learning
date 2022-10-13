# Parser and reward calculator for joined binary schema v2 files

## File format

The file format constitute of a sequence of messages writen one after the other. All messages have the same format:

- 4 bytes - message type
- 4 bytes - payload size or inline payload.
- <size> bytes - payload content _(optional)_
- padding bytes - size % 8 bytes to ensure message alignment _(optional)_

The size of all message must be aligned to 8 bytes. Paddings bytes are inserted at the end and should not be included in the payload size.
Padding bytes should be zero but is not enforced.

### Message types

Each message has an unique indentifier, those are the ones currently recognized:

- `MSG_TYPE_FILEMAGIC = 0x42465756 //'VWFB'`
- `MSG_TYPE_HEADER = 0x55555555`
- `MSG_TYPE_REGULAR = 0xFFFFFFFF`
- `MSG_TYPE_CHECKPOINT = 0x11111111`
- `MSG_TYPE_EOF = 0xAAAAAAAA`

### Message payloads

For all flatbuffer payloads see `rlclientlib/schema/v2`

### File Magic message

The payload is inline and it's the file format version.

The only value accepted is `1`.

This message should be the first on a file, making it easy to recognize files following its format by their 4 bytes watermark.

### Header message

Payload is a flatbuffer message of type `FileHeader` (see `FileFormat.fbs`) .

This message contains informational data about this file. It should include provenance
details such as how it was generated, the version and parameters of the program used, generation
time and other information that helps troubleshooting.

### Checkpoint message

Payload is a flatbuffer message of type `CheckpointInfo` (see `FileFormat.fbs`).

This message includes information on how to join events comming after it.
Checkpoint messages are the recomended point to split larger files at as they contain all info
required to process the stream that follows them.

### Regular message

Payload is a flatbuffer message of type `JoinedPayload` (see `FileFormat.fbs`).

This message include multiple events, sharing one or more event-ids that should be processed together.

### Recomended message ordering

The recomended ordering of messages in a file is the following:

- 1 file magic message
- 1 file header message
N times:
- 1 checkpoint message
- M regular messages


## Linux

### Build Linux

**Note**: To statically link set `-DSTATIC_LINK_BINARY_PARSER=ON` during `cmake`

from `external_parser`:

- mkdir build
- cd build
- cmake ..
- make -j $(nproc)

vw executable located at: `external_parser/build/vw`

### Run

`./vw -d <file> --binary_parser [other vw args]`


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
- cmake .. -DCMAKE_TOOLCHAIN_FILE=<VCPKG_INSTALLATION_ROOT>\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -G "Visual Studio 15 2017" -A x64 -DBUILD_FLATBUFFERS=OFF -DCMAKE_CONFIGURATION_TYPES="Release" -DWARNINGS=OFF -DWARNINGS=OFF -DWARNING_AS_ERROR=OFF -DDO_NOT_BUILD_VW_C_WRAPPER=OFF  -DBUILD_JAVA=OFF -DBUILD_PYTHON=OFF -DBUILD_TESTING=OFF -DBUILD_EXPERIMENTAL_BINDING=OFF
- <MSBUILD_PATH> /verbosity:normal /m /p:Configuration=Release;Platform=x64 vw_binary_parser.sln

`vw_binary_parser.sln` will be available under `build` and can be used to open the solution in visual studio

vw executable located at: `external_parser\build\Release\vw.exe`

### Run

`.\Release\vw.exe -d <file> --binary_parser [other vw args]`
