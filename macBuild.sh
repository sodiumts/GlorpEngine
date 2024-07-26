#!/bin/bash

BUILD_TYPE="Debug"
BUILD_APP_BUNDLE="OFF"

usage() {
    echo "Usage: $0 [-r]"
    echo "  -r    Build in release mode and create an app bundle for macOS"
    exit 1
}

while getopts "r" opt; do
    case ${opt} in
        r)
            BUILD_TYPE="Release"
            BUILD_APP_BUNDLE="ON"
            ;;
        *)
            usage
            ;;
    esac
done

echo "Release bundle build = ${BUILD_APP_BUNDLE}"
echo "Build type = ${BUILD_TYPE}"

mkdir -p build
cd build
cmake -S ../ -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_APP_BUNDLE=${BUILD_APP_BUNDLE} -B .
make -j && make Shaders

if [ "${BUILD_APP_BUNDLE}" = "OFF" ]; then
    cd .. && ./build/GlorpEngine
fi
