if not exist build mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -S ../ -B . -G "MinGW Makefiles"
mingw32-make.exe -j && mingw32-make.exe Shaders
cd ..
