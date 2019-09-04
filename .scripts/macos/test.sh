#!/bin/bash

set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../

cd $REPO_DIR/build

# Run unit test suite
./unit_test/rltest --log_format=JUNIT --log_sink=TEST-rlclientlib.xml
