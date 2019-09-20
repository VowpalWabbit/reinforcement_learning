#!/bin/bash

set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../

cd $REPO_DIR/build

# Run unit test suite

# Cannot use JUNIT as this is not supported in Boost 1.58
# ./unit_test/rltest --log_format=JUNIT --log_sink=TEST-rlclientlib.xml

export CTEST_OUTPUT_ON_FAILURE=1
make test
