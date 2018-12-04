[![Build Status](https://travis-ci.org/VowpalWabbit/reinforcement_learning.svg?branch=master)](https://travis-ci.org/VowpalWabbit/reinforcement_learning)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/57p7o5v34onsqma2/branch/master?svg=true)](https://ci.appveyor.com/project/JohnLangford/reinforcement-learning/branch/master)

# reinforcement_learning

## Dependencies

### Ubuntu

```
sudo apt-get install libboost-all-dev
```

Cpprest needs to be installed:
```
cd ~
git clone https://github.com/Microsoft/cpprestsdk.git cpprestsdk
cd cpprestsdk/Release
# Checkout 2.10.1 version of cpprestsdk
git checkout e8dda215426172cd348e4d6d455141f40768bf47
mkdir build
cd build
cmake ..
make
make install
cd ../../../
rm -rf cpprestsdk
```

### Windows
Windows dependencies are managed through Vcpkg and Nuget.

```
vcpkg install cpprestsdk:x64-windows
vcpkg install zlib:x64-windows
vcpkg install boost-system:x64-windows
vcpkg install boost-program-options:x64-windows
vcpkg install boost-test:x64-windows
vcpkg install boost-uuid:x64-windows
```

## Build

```
mkdir build
cd build
cmake ..
make

# Test
mkdir build
cd build
cmake .. -DTURN_OFF_DEVIRTUALIZE=On
make rltest
make test
```

### Experimental - CMake on Windows
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
