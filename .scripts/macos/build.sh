#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

mkdir -p build
cd build

cmake .. -DTURN_OFF_DEVIRTUALIZE=On
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j${NUM_PROCESSORS}
