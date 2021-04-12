# Parser and reward calculator for joined binary schema v2 files

## Linux

### Build Linux

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