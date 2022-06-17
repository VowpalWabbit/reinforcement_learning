#!/bin/bash
set -e
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$SCRIPT_DIR/../../"
cd "$REPO_DIR"

# -DCMAKE_EXPORT_COMPILE_COMMANDS=On is manually set because in CMake 3.16, just setting it in the CMakeLists.txt does not work.
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=On
cd build

# check that compile_commands.json was generated
if [[ ! -f compile_commands.json ]]; then
    echo "Error: CMake did not successfully generate compile_commands.json"
    exit 1
fi

# remove entries in compile_commands.json referencing ext_libs
jq 'map(select(.directory | contains("ext_libs") or contains("unit_test") | not))' compile_commands.json > compile_commands_without_ext_libs.json
mv compile_commands_without_ext_libs.json compile_commands.json

# run clang-tidy on the modified compile_commands.json
cd ..
run-clang-tidy -p build
