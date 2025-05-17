#!/bin/bash

BUILD_TYPE="Debug"
BUILD_APP_BUNDLE="ON"

echo "Release bundle build = ${BUILD_APP_BUNDLE}"
echo "Build type = ${BUILD_TYPE}"

mkdir -p build
cd build
cmake -S ../ -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_APP_BUNDLE=${BUILD_APP_BUNDLE} -B .
make -j && make Shaders
