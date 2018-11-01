#!/bin/bash

set -e

# Build reinforcement_learning
mkdir -p build
cd build
cmake ..
NUM_PROCESSORS=$(cat nprocs.txt)
make -j${NUM_PROCESSORS}

export CTEST_OUTPUT_ON_FAILURE=1
make test
