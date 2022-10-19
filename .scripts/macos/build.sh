#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../
cd $REPO_DIR

# On MacOS CMake is unable to find brew installed OpenSSL, so we need to pass it where to look.
OPEN_SSL_DIR=`brew --prefix openssl`

cmake -S . -B build -G Ninja -DOPENSSL_ROOT_DIR=$OPEN_SSL_DIR -DOPENSSL_LIBRARIES=$OPEN_SSL_DIR/lib -DVCPKG_MANIFEST_MODE=Off
cmake --build build --target all
