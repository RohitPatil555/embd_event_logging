#!/bin/bash
set -e

EXAMPLE_BUILD_OUT="build/example"
ROOT_DIR="$PWD"

# Build example
mkdir -p ${EXAMPLE_BUILD_OUT}
cd ${EXAMPLE_BUILD_OUT}
cmake ../../example
make

# execute to get event stream.
./example

# prepare folder for debugging trace using babeltrace2
# here we will only read event on console.
echo "=========================================================================================="
echo "Demonstrate babeltrace output ......"
echo "=========================================================================================="
mkdir trace_config
cp generated/metadata trace_config/
cp stream.bin trace_config/
babeltrace2 trace_config/

cd ${ROOT_DIR}
