#!/bin/bash

BUILD_TYPE="Debug"

echo "Build type = ${BUILD_TYPE}"

mkdir -p build
cd build
cmake -S ../ \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DSDL_VIDEO=ON \
  -DSDL_WAYLAND=ON \
  -DSDL_X11=ON -B .
make -j && make Shaders

cd .. && SDL_VIDEODRIVER=wayland ./build/GlorpEngine
