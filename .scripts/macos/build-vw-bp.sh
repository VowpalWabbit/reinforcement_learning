#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR=$SCRIPT_DIR/../../

cd $REPO_DIR/external_parser

cmake -S . -B build -G Ninja
cmake --build build --target vw-bin binary_parser_unit_tests
