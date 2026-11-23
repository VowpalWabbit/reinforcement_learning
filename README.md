[![Linux Build Status)](https://img.shields.io/azure-devops/build/vowpalwabbit/3934113c-9e2b-4dbc-8972-72ab9b9b4342/23/master?label=Linux%20build&logo=Azure%20Devops)](https://dev.azure.com/vowpalwabbit/Vowpal%20Wabbit/_build?definitionId=31)
[![MacOS Build Status](https://img.shields.io/azure-devops/build/vowpalwabbit/3934113c-9e2b-4dbc-8972-72ab9b9b4342/22/master?label=MacOS%20build&logo=Azure%20Devops)](https://dev.azure.com/vowpalwabbit/Vowpal%20Wabbit/_build?definitionId=32)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/57p7o5v34onsqma2/branch/master?svg=true)](https://ci.appveyor.com/project/JohnLangford/reinforcement-learning/branch/master)
[![Integration with latest VW](https://github.com/VowpalWabbit/reinforcement_learning/actions/workflows/daily_integration.yml/badge.svg?event=schedule)](https://github.com/VowpalWabbit/reinforcement_learning/actions/workflows/daily_integration.yml)

# RL Client Library
Interaction-side integration library for Reinforcement Learning loops: Predict, Log, [Learn,] Update

## Compiling the library
### Getting the source code
```sh
git clone --recursive https://github.com/VowpalWabbit/reinforcement_learning.git
cd reinforcement_learning
```

### Building with Vcpkg + CMake Presets
#### Requirements
* CMake 3.21+ (CMake Presets schema version 3; CI pins CMake 3.31.6)
* Ninja
* A C++14-capable compiler (the pinned Vowpal Wabbit 9.11.9 requires C++14)

Optional Python bindings use pybind11 3.1.0 and require Python 3.8 or newer.
Enable them with `-DRL_BUILD_PYTHON=ON`; select an interpreter with
`-DPython_EXECUTABLE=<path-to-python>`.
The checked-in vcpkg revision and manifest baseline are both pinned to
2025.02.14, with fmt 11.0.2 and spdlog 1.15.0 overrides.
Windows builds apply small overlays to that pinned registry: the Boost helper
avoids trailing separators in directory renames rejected by Windows, and the
CMake helper corrects the native MinGW package filter so acquiring pkgconf does
not unnecessarily fetch a removed MSYS runtime archive. Package versions and
download hashes remain pinned.
Root and standalone parser CMake builds select it automatically; direct Windows
vcpkg installs must add `--overlay-ports=cmake\vcpkg-overlay-ports`.

#### Commands
```sh
cmake --preset=vcpkg-release
cmake --build --preset=vcpkg-release
```

## Alternative system-specific build instructions
### Ubuntu
#### Dependencies
Install the dependencies for this project with the following commands. We recommend the use of [vcpkg](https://github.com/microsoft/vcpkg) for installing `cpprestsdk` and `flatbuffers`.
```sh
sudo apt-get install libboost-all-dev libssl-dev
vcpkg install cpprestsdk flatbuffers
```

#### Configure + Build
When configuring a CMake project using vcpkg dependencies, we must provide the full path to the `vcpkg.cmake` toolchain file.
```sh
# Configure
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=</path/to/vcpkg>/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build -j `nproc`

# Test
cmake --build build --target rltest -j `nproc`
cmake --build build --target test
```

### MacOS
#### Dependencies
MacOS dependencies can be managed through homebrew.
```sh
brew install cpprestsdk flatbuffers openssl
```

#### Configure +  Build
In order to build using homebrew dependencies, you must invoke cmake this way:
```sh
cmake -S . -B build -DOPENSSL_ROOT_DIR=`brew --prefix openssl` -DOPENSSL_LIBRARIES=`brew --prefix openssl`/lib
cmake --build build --target all -j 4
```

### Windows
#### Dependencies
Dependencies on Windows should be managed using [vcpkg](https://github.com/microsoft/vcpkg).
```cmd
vcpkg install --triplet x64-windows zlib boost-system boost-program-options boost-test boost-align boost-foreach boost-math boost-uuid cpprestsdk flatbuffers openssl
```

If needed, add the flatbuffers executables to your PATH: `<vcpkg_root>\installed\x64-windows\tools\flatbuffers`

#### Configure + Build in Visual Studio
Open the project directory in Visual Studio. Building the library can be easily done in the GUI.
- Edit CMake command line settings with `Project > CMake Settings for <project name>`
- Run CMake configuration with `Project > Configure Cache`. Use `Project > Delete Cache and Reconfigure` to force a full reconfiguration starting from a clean working directory.
- Compile with `Build > Build All`
- Run tests with `Test > Run CTests for <project name>`

This procedure has been verified to work well in Visual Studio 2022.
The Visual Studio generator selects an installed Windows SDK by default.
Use `-DCMAKE_SYSTEM_VERSION=<sdk-version>` to select a specific installed SDK.

#### Configure with command line + Build in Visual Studio
Alternatively, CMake configuration can be done in the command line.
```cmd
cmake -S . -B build  -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -A x64 -G "Visual Studio 16 2019"
rem Generates a solution you can open and use in Visual Studio
.\build\reinforcement_learning.sln
```

Set VcpkgIntegration environment variable to `vcpkg.targets` file on your machine. Example:
```
VcpkgIntegration=c:\s\vcpkg\scripts\buildsystems\msbuild\vcpkg.targets
```

Open `reinforcement_learning.sln` in Visual Studio. Build Release or Debug x64 configuration.

## Troubleshooting
### Python serialized-event tools
The E2E comparator and log parser tools generate their Python FlatBuffers
accessors on first use. Use the pinned `flatc` 23.1.21 on `PATH`, or set `FLATC`
to its executable path. Generation goes through binary reflection schemas
(`--binary --schema`, then `--python`) to preserve the wire schema's literal
string unions, which direct Python schema parsing rejects. The comparison
still decodes and checks both numeric and literal outcomes.

### Long Windows build paths
Windows compiler and resource tools can fail on deeply nested vcpkg build paths
even when Git long paths are enabled. Keep the checkout path short, or pass
`-DVCPKG_INSTALL_OPTIONS="--x-buildtrees-root=<short-absolute-path>"` when
configuring to move dependency scratch files to a shorter path. The installed
dependency directory is unchanged. Direct vcpkg installs accept the same
`--x-buildtrees-root` option.

### OpenSSL on MacOS
If you get an error similar to the following on MacOS when running `cmake ..`, then you may be able to fix it by supplying the OpenSSL path to CMake.
```
Make Error at /usr/local/Cellar/cmake/3.14.4/share/cmake/Modules/FindPackageHandleStandardArgs.cmake:137 (message):
  Could NOT find OpenSSL, try to set the path to OpenSSL root folder in the
  system variable OPENSSL_ROOT_DIR (missing: OPENSSL_INCLUDE_DIR)
Call Stack (most recent call first):
  /usr/local/Cellar/cmake/3.14.4/share/cmake/Modules/FindPackageHandleStandardArgs.cmake:378 (_FPHSA_FAILURE_MESSAGE)
  /usr/local/Cellar/cmake/3.14.4/share/cmake/Modules/FindOpenSSL.cmake:413 (find_package_handle_standard_args)
  /usr/local/Cellar/cmake/3.14.4/share/cmake/Modules/CMakeFindDependencyMacro.cmake:48 (find_package)
  /usr/local/lib/cpprestsdk/cpprestsdk-config.cmake:11 (find_dependency)
  CMakeLists.txt:9 (find_package)
```

This can be fixed by invoking CMake similar to the following:
```bash
cmake -DOPENSSL_ROOT_DIR=`brew --prefix openssl` -DOPENSSL_LIBRARIES=`brew --prefix openssl`/lib ..
```

### System cpprestsdk on Linux
Installing cpprestsdk on Ubuntu18.04 using apt-get may result in cmake failing with:
```
CMake Error at CMakeLists.txt:9 (find_package):
  By not providing "Findcpprestsdk.cmake" in CMAKE_MODULE_PATH this project
  has asked CMake to find a package configuration file provided by
  "cpprestsdk", but CMake did not find one.

  Could not find a package configuration file provided by "cpprestsdk" with
  any of the following names:

    cpprestsdkConfig.cmake
    cpprestsdk-config.cmake

  Add the installation prefix of "cpprestsdk" to CMAKE_PREFIX_PATH or set
  "cpprestsdk_DIR" to a directory containing one of the above files.  If
  "cpprestsdk" provides a separate development package or SDK, be sure it has
  been installed.
```

The workaround is to specify where to search
```
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake
```
