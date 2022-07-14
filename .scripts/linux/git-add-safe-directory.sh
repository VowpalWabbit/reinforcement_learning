#!/bin/bash

# Fix git "unsafe repository" error
# https://github.com/actions/checkout/issues/766
# https://stackoverflow.com/questions/71855882/how-to-add-directory-recursively-on-git-safe-directory
git config --global --add safe.directory '*'
