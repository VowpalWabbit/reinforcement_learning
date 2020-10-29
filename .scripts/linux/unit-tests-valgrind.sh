#!/bin/bash

set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../

cd $REPO_DIR/build

# Run unit test suite with valgrind
valgrind  --quiet --error-exitcode=100 --leak-check=full ./unit_test/rltest -- valgrind
