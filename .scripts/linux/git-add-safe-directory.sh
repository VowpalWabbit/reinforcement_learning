#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$SCRIPT_DIR/../../"
cd "$REPO_DIR"

# Fix git "unsafe repository" error
# https://github.com/actions/checkout/issues/766
if [[ -n "$GITHUB_WORKSPACE" ]]; then
    echo "Adding safe directory: $GITHUB_WORKSPACE"
    git config --global --add safe.directory "$GITHUB_WORKSPACE"
fi

# add ext_libs
[[ -d ext_libs ]] || exit
cd ext_libs
echo "Adding safe directory: $(pwd)"
git config --global --add safe.directory "$(pwd)"

# add ext_libs/vowpal_wabbit
[[ -d vowpal_wabbit ]] || exit
cd vowpal_wabbit
echo "Adding safe directory: $(pwd)"
git config --global --add safe.directory "$(pwd)"
