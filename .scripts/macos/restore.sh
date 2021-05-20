#!/bin/bash

set -e
set -x

brew update
brew upgrade

brew install cmake boost cpprestsdk openssl
# install flatbuffers 1.10.0
brew install https://raw.githubusercontent.com/Homebrew/homebrew-core/050eed3eee550b0489c66fa9d6ca5991baf9268e/Formula/flatbuffers.rb
