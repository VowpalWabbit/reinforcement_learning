#!/bin/bash

set -e
set -x

sudo apt remove --yes --force-yes cmake

# Upgrade CMake
version=3.5
build=2
mkdir ~/temp
cd ~/temp
wget https://cmake.org/files/v$version/cmake-$version.$build-Linux-x86_64.sh
sudo mkdir /opt/cmake
sudo sh cmake-$version.$build-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
sudo ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

cd /reinforcement_learning

# Build reinforcement_learning
mkdir -p build
cd build
cmake .. -DTURN_OFF_DEVIRTUALIZE=On
NUM_PROCESSORS=$(cat nprocs.txt)
make -j${NUM_PROCESSORS}

export CTEST_OUTPUT_ON_FAILURE=1
make test
