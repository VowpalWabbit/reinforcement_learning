[![Build Status](https://travis-ci.org/VowpalWabbit/reinforcement_learning.svg?branch=master)](https://travis-ci.org/VowpalWabbit/reinforcement_learning)
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
make
make install
cd ../../../
rm -rf cpprestsdk
```


#### Install Flatbuffers
```bash
cd ~
git clone https://github.com/google/flatbuffers.git flatbuffers
cd ./flatbuffers
# 1.10.0 release commit
git checkout 925c1d77fcc72636924c3c13428a34180c30f96f
mkdir build
cd build
cmake .. -DFLATBUFFERS_BUILD_TESTS=Off -DFLATBUFFERS_INSTALL=On -DCMAKE_BUILD_TYPE=Release -DFLATBUFFERS_BUILD_FLATHASH=Off
make
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
cmake -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl/1.0.2r -DOPENSSL_LIBRARIES=/usr/local/Cellar/openssl/1.0.2r/lib ..
```

### Test
```
mkdir build
cd build
cmake .. -DTURN_OFF_DEVIRTUALIZE=On
make rltest
make test
```

## Windows
Windows dependencies are managed through Vcpkg and Nuget.

```
vcpkg install cpprestsdk:x64-windows
vcpkg install flatbuffers:x64-windows
```

### Build + Test

Open `rl.sln` in Visual Studio 2017.

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
