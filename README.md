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
```

## Building

You should be able to build the *vowpal wabbit* on most systems with:
```
make all

# Test
make clean
make rl_clientlib_test
```
