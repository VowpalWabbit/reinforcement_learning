#!/bin/bash

set -e
set -x
# https://github.com/actions/virtual-environments/issues/1811
brew uninstall openssl@1.0.2t
brew install cmake boost cpprestsdk flatbuffers openssl
