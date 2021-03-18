#!/bin/bash

set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../

cd $REPO_DIR/external_parser/build

# Run unit test suite for external parser

export CTEST_OUTPUT_ON_FAILURE=1
make test
