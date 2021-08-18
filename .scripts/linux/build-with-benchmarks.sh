#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

rm -rf build
mkdir build
cd build
cmake .. -DBUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
NUM_PROCESSORS=$(cat nprocs.txt)
make rl_benchmarks -j ${NUM_PROCESSORS}
