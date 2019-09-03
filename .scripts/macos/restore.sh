#!/bin/bash

set -e
set -x

# Install specifc version of OpenSSL as we need to refer to the install directory specifically
brew install cmake boost cpprestsdk flatbuffers openssl@1.0.2r
