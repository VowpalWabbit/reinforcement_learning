#!/bin/bash

set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR/external_parser

# Build reinforcement_learning/external_parser
cmake -S . -B build -G Ninja \
    -DSTATIC_LINK_BINARY_PARSER=ON \
    -DFMT_SYS_DEP=ON \
    -DSPDLOG_SYS_DEP=ON

cmake --build build --target all
