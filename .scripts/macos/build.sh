#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

mkdir -p build
cd build

# On MacOS CMake is unable to find brew installed OpenSSL, so we need to pass it where to look.
OPEN_SSL_DIR="/usr/local/opt/openssl"

cmake .. -DTURN_OFF_DEVIRTUALIZE=On -DOPENSSL_ROOT_DIR=$OPEN_SSL_DIR -DOPENSSL_LIBRARIES=$OPEN_SSL_DIR/lib
NUM_PROCESSORS=$(cat nprocs.txt)
make all -j${NUM_PROCESSORS} VERBOSE=1
