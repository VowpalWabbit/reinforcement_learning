#!/bin/bash

set -e

: ${VCPKG_ROOT?"VCPKG_ROOT environment variable must be set with the root of vcpkg"}
: ${RL_PYTHON_EXT_DEPS?"RL_PYTHON_EXT_DEPS environment variable must be set with the path to save dependencies to"}

DEPS_DIR=$RL_PYTHON_EXT_DEPS
mkdir -p $DEPS_DIR

cp $VCPKG_ROOT/installed/x64-linux/lib/libcpprest.a $DEPS_DIR
cp $VCPKG_ROOT/installed/x64-linux/lib/libboost_system.a $DEPS_DIR
cp $VCPKG_ROOT/installed/x64-linux/lib/libboost_program_options.a $DEPS_DIR
cp $VCPKG_ROOT/installed/x64-linux/lib/libssl.a $DEPS_DIR
cp $VCPKG_ROOT/installed/x64-linux/lib/libcrypto.a $DEPS_DIR
cp $VCPKG_ROOT/installed/x64-linux/lib/libz.a $DEPS_DIR
