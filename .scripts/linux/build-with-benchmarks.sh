#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

rm -rf build
cmake -S . -B build -G Ninja -DRL_BUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release -DVCPKG_MANIFEST_MODE=Off
cmake --build build --target rl_benchmarks