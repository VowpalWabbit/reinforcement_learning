#!/bin/bash

set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# Build reinforcement_learning
cmake -S . -B build -G Ninja \
    -Drlclientlib_BUILD_ONNXRUNTIME_EXTENSION=On \
    -DFMT_SYS_DEP=ON \
    -DSPDLOG_SYS_DEP=ON

# Only difference is only the rltest target is built
cmake --build build --target rltest
