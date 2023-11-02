#!/bin/bash
set -e
set -x

if [[ $# > 1 || ($# == 1 && "$1" != "fix") ]]; then
    echo "Usage: \"$0" to run clang-tidy check 
    echo "    or \"$0 fix\" to run clang-tidy and automatically apply fixes"
    exit 1
fi
[[ $# == 1 && "$1" == "fix" ]] && APPLY_FIX=1

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$SCRIPT_DIR/../../"
cd "$REPO_DIR"

# -DCMAKE_EXPORT_COMPILE_COMMANDS=On is manually set because in CMake 3.16, just setting it in the CMakeLists.txt does not work.
cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=On

# generate flatbuffers files
cmake --build build --target fbgen

# check that compile_commands.json was generated
cd build
if [[ ! -f compile_commands.json ]]; then
    echo "Error: CMake did not successfully generate compile_commands.json"
    exit 1
fi

# remove entries in compile_commands.json referencing ext_libs 
jq 'map(select(.directory | contains("ext_libs") or contains("unit_test") | not))' compile_commands.json > compile_commands_without_ext_libs.json
mv compile_commands_without_ext_libs.json compile_commands.json
cd ..

# run clang-tidy on the modified compile_commands.json
if [[ $APPLY_FIX ]]; then
    echo "Running clang-tidy and applying fixes..."
    run-clang-tidy -fix -p build
    echo "Done. Run clang-format.sh to format any applied fixes and maintain consistent style."
else
    run-clang-tidy -p build
fi
