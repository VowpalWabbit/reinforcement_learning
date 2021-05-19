#!/bin/bash

set -e
set -x

brew update

brew install cmake boost cpprestsdk flatbuffers openssl
