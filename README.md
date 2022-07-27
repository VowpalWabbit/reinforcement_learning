[![Linux Build Status)](https://img.shields.io/azure-devops/build/vowpalwabbit/3934113c-9e2b-4dbc-8972-72ab9b9b4342/23/master?label=Linux%20build&logo=Azure%20Devops)](https://dev.azure.com/vowpalwabbit/Vowpal%20Wabbit/_build?definitionId=31)
[![MacOS Build Status](https://img.shields.io/azure-devops/build/vowpalwabbit/3934113c-9e2b-4dbc-8972-72ab9b9b4342/22/master?label=MacOS%20build&logo=Azure%20Devops)](https://dev.azure.com/vowpalwabbit/Vowpal%20Wabbit/_build?definitionId=32)
[![Integration with latest VW](https://github.com/VowpalWabbit/reinforcement_learning/actions/workflows/daily_integration.yml/badge.svg?event=schedule)](https://github.com/VowpalWabbit/reinforcement_learning/actions/workflows/daily_integration.yml)

# RL Client Library
Interaction-side integration library for Reinforcement Learning loops: Predict, Log, [Learn,] Update

## Compiling the library
### Getting the source code
```sh
git clone https://github.com/VowpalWabbit/reinforcement_learning.git
git submodule update --init --recursive
```

### Dependencies
Install the dependencies for this project with the following commands. We recommend the use of [vcpkg](https://github.com/microsoft/vcpkg).

Linux/MacOS
```sh
vcpkg install cpprestsdk flatbuffers boost-test boost-system boost-uuid boost-program-options boost-thread boost-math openssl zstd zlib fmt spdlog rapidjson
```

Windows
```
vcpkg install --triplet x64-windows cpprestsdk flatbuffers boost-test boost-system boost-uuid boost-program-options boost-thread boost-math openssl zstd zlib fmt spdlog rapidjson
```

### Configure + Build
When configuring a CMake project using vcpkg dependencies, we must provide the full path to the `vcpkg.cmake` toolchain file.
```sh
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=</path/to/vcpkg>/scripts/buildsystems/vcpkg.cmake -DRAPIDJSON_SYS_DEP=On -DFMT_SYS_DEP=On -DSPDLOG_SYS_DEP=On -DVW_BOOST_MATH_SYS_DEP=On
```

Using `-G Ninja` is recommended as it provides automatic build parallelization.

> Note: If your dependencies are static (e.g. MacOS vcpkg by default), then you will need to pass `-DRL_INTERNAL_BOOST_TEST_IS_SHARED_DEP=Off` to `cmake` when configuring.

### Build

```sh
# Build
cmake --build build
```

### Test
```sh
ctest --test-dir build
```

### Configure + Build in Visual Studio
Open the project directory in Visual Studio. Building the library can be easily done in the GUI.
- Edit CMake command line settings with `Project > CMake Settings for <project name>`
- Run CMake configuration with `Project > Configure Cache`. Use `Project > Delete Cache and Reconfigure` to force a full reconfiguration starting from a clean working directory.
- Compile with `Build > Build All`
- Run tests with `Test > Run CTests for <project name>`

This procedure has been verified to work well in Visual Studio 2022.

### Configure with command line + Build in Visual Studio

Follow instructions in [configure](#configure) but also include:
```
-DVCPKG_TARGET_TRIPLET=x64-windows -A x64 -G "Visual Studio 16 2019"
```

Or, change to whatever generator you wish.

Then, open the generated solution in the build directory.