#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$SCRIPT_DIR/../../"
cd "$REPO_DIR"

# -DCMAKE_EXPORT_COMPILE_COMMANDS=On is manually set because in CMake 3.16, just setting it in the CMakeLists.txt does not work.
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=On
run-clang-tidy -p build -header-filter=vw/*
