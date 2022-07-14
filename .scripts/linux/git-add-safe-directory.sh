#!/bin/bash

# Fix git "unsafe repository" error
# https://github.com/actions/checkout/issues/766
if [[ -n "$GITHUB_WORKSPACE" ]]; then
    git config --global --add safe.directory "$GITHUB_WORKSPACE"
fi
