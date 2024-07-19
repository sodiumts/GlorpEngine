mkdir -p build
cd build
cmake -S ../ -DCMAKE_BUILD_TYPE=Debug -B .
make -j && make Shaders && ./GlorpEngine
cd ..
