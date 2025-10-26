#!/usr/bin/env bash

set -e

EXEC_DIR=$PWD
BUILD_DIR=build
COPY_ARTIFACTS=false


if [ "$GITHUB_ACTIONS" = "true" ]; then
    BUILD_DIR=/tmp/build
    COPY_ARTIFACTS=true
fi

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
cd tests/
ctest --output-on-failure
cd -

if [ "$COPY_ARTIFACTS" = "true" ]; then
    sudo cp -r /tmp/build/docs_output  /workspaces/artifacts/
    echo "Copied Artifacts Done"
fi

cd $EXEC_DIR

./example/build_example.sh
