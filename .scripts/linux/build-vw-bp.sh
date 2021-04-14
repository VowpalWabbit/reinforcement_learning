#!/bin/bash

set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR/external_parser

# Build reinforcement_learning/external_parser
mkdir -p build
cd build
cmake .. -DSTATIC_LINK_BINARY_PARSER=ON
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j${NUM_PROCESSORS}
