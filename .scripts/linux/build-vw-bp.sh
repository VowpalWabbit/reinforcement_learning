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
    -DSPDLOG_SYS_DEP=ON \
    -DVCPKG_MANIFEST_MODE=Off

cmake --build build --target rl_binary_parser_bin binary_parser_unit_tests
