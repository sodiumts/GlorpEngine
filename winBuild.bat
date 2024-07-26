mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
mingw32-make.exe -j4
mingw32-make.exe Shaders
cd ..
