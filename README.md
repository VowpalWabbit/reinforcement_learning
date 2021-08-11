[![Linux Build Status)](https://img.shields.io/azure-devops/build/vowpalwabbit/3934113c-9e2b-4dbc-8972-72ab9b9b4342/23/master?label=Linux%20build&logo=Azure%20Devops)](https://dev.azure.com/vowpalwabbit/Vowpal%20Wabbit/_build?definitionId=31)
[![MacOS Build Status](https://img.shields.io/azure-devops/build/vowpalwabbit/3934113c-9e2b-4dbc-8972-72ab9b9b4342/22/master?label=MacOS%20build&logo=Azure%20Devops)](https://dev.azure.com/vowpalwabbit/Vowpal%20Wabbit/_build?definitionId=32)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/57p7o5v34onsqma2/branch/master?svg=true)](https://ci.appveyor.com/project/JohnLangford/reinforcement-learning/branch/master)

# reinforcement_learning

```
git submodule update --init --recursive
```

## Ubuntu

### Dependencies

```
sudo apt-get install libboost-all-dev libssl-dev
```

#### Install Cpprest
```bash
cd ~
git clone https://github.com/Microsoft/cpprestsdk.git cpprestsdk
cd cpprestsdk
# Checkout 2.10.1 version of cpprestsdk
git checkout e8dda215426172cd348e4d6d455141f40768bf47
git submodule update --init
cd Release
mkdir build
cd build
cmake ..
make -j `nproc`
make install
cd ../../../
rm -rf cpprestsdk
```


#### Install Flatbuffers
```bash
cd ~
git clone https://github.com/google/flatbuffers.git flatbuffers
cd ./flatbuffers
# 2.0.0 release commit
git checkout a9a295fecf3fbd5a4f571f53b01f63202a3e2113
mkdir build
cd build
cmake .. -DFLATBUFFERS_BUILD_TESTS=Off -DFLATBUFFERS_INSTALL=On -DCMAKE_BUILD_TYPE=Release -DFLATBUFFERS_BUILD_FLATHASH=Off
make -j `nproc`
sudo make install
cd ../../
rm -rf flatbuffers
```

### Build

```
mkdir build
cd build
cmake ..
make
```

#### Troubleshooting
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

### Test
```
mkdir build
cd build
cmake ..
make rltest
make test
```

## Windows
Windows dependencies are managed through Vcpkg and Nuget.

```
vcpkg install cpprestsdk:x64-windows
vcpkg install flatbuffers:x64-windows
```

You'll need to add the flatbuffers tool directory to your PATH aswell: `<vcpkg_root>\installed\x64-windows\tools\flatbuffers`

## MacOS

MacOS dependencies can be managed through homebrew.

```
brew install cpprestsdk flatbuffers openssl
```

In order to build using homebrew dependencies, you must invoke cmake this way:

```
mkdir build
cd build
cmake -DOPENSSL_ROOT_DIR=`brew --prefix openssl` -DOPENSSL_LIBRARIES=`brew --prefix openssl`/lib ..
```


### Build + Test

Set VcpkgIntegration environment variable to vcpkg.targets file on your machine

Example:
VcpkgIntegration=c:\s\vcpkg\scripts\buildsystems\msbuild\vcpkg.targets

Ensure that the v140 toolset is installed in Visual Studio 2017.

Open `rl.sln` in Visual Studio 2017.
Build Release or Debug x64 configuration.

### Experimental - CMake on Windows (Do not mix with checked in solution)
Using CMake is an alternate way to configure and build the project. Currently it only supports the C++ projects.
All dependencies are managed through Vcpkg:
```
# Warning only use if generating solution with CMake)
vcpkg install cpprestsdk:x64-windows
vcpkg install zlib:x64-windows
vcpkg install boost-system:x64-windows
vcpkg install boost-program-options:x64-windows
vcpkg install boost-test:x64-windows
vcpkg install boost-uuid:x64-windows
```

Run the following to generate a solution file to open in Visual Studio 2017:
```
mkdir build
cd build
cmake .. -G "Visual Studio 15 2017 Win64" -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]\scripts\buildsystems\vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
.\reinforcement_learning.sln
```

## Make targets
- `doc` - Python and C++ docs
- `_rl_client` - Python bindings
- `rlclientlib` - rlclient library
- `rltest` - unit tests
