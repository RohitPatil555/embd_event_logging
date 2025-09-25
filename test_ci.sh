#!/usr/bin/env bash

set -e

BUILD_DIR=/tmp/build

# clean build directory
rm -rf ${BUILD_DIR}
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}

# config space
cmake /workspaces/embd_event_logging -DCMAKE_BUILD_TYPE=Debug

# build cmake
cmake --build . -- -j$(nproc)

# build docs
cmake --build . --target docs

# Run test
ctest --output-on-failure
