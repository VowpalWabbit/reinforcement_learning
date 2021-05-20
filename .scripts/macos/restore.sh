#!/bin/bash

set -e
set -x

brew update
brew upgrade

brew install cmake boost cpprestsdk flatbuffers@1.10.0 openssl
