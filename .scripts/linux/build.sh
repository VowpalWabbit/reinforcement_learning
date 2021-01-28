#!/bin/bash

set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Build reinforcement_learning
mkdir -p build
cd build
cmake .. -Drlclientlib_BUILD_ONNXRUNTIME_EXTENSION=On
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j${NUM_PROCESSORS}
