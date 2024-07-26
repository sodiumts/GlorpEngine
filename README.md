# Glorp Engine
Glorp engine is a Vulkan / GLFW based rendering engine for MacOS and Windows written in C++
![image](https://github.com/user-attachments/assets/599ccd99-398f-498f-baaf-b8c8d033a70b)

*Model and textures created by [nigelgoh](https://sketchfab.com/nigelgoh) [(CC BY 4.0)](https://web.archive.org/web/20200428202538/https://sketchfab.com/3d-models/viking-room-a49f1b8e4f5c4ecf9e1fe7d81915ad38)*
## Prerequisites For Building

**Clone the repository using with recurse submodules**
Make sure you have the following tools installed on your system:
### General requirements
* **CMake** (version 3.29.0 or later)
### Platform-specific requirements
**Windows**
* **MinGW** (for MinGW Makefile generation)
  * Ensure `MINGW_PATH` is set in `.env.cmake`

**MacOS**
* **Xcode command line tools**
### Libraries
* **Vulkan SDK**: Make sure Vulkan is installed and `VULKAN_SDK_PATH` is set in `.env.cmake` if using a specific version. Consult [LunarG](https://vulkan.lunarg.com/) for the Vulkan SDK installation.
* **GLFW**: GLFW should get installed by CMake, if you wish to use a specific version, set `GLFW_PATH` in `.env.cmake`.
* **tinyOBJLoader**: The git repository containts `tiny_obj_loader.h` header in `external/`. If you wish to provide your own specific version and specify `TINYOBJ_PATH` in `.env.cmake`.
* **Dear ImGui**: The external folder should contain the git submodule for Dear ImGui.
## Environment Setup
### .env.cmake
Create a `.env.cmake` file in the project root and specify the necessary dependency paths:
```cmake
# .env.cmake
set(MINGW_PATH "C:/path/to/mingw")
set(VULKAN_SDK_PATH "C:/path/to/vulkan")
set(GLFW_PATH "C:/path/to/glfw")
set(TINYOBJ_PATH "path/to/tinyobjloader") # Optional if using the included version
```

## Building The Project
Make sure you have all the required dependencies installed on your system.
1. Clone the repository with all the git submodules.
```sh
git clone --recurse-submodules https://github.com/sodiumts/GlorpEngine
cd GlorpEngine
```
2. Run either `macBuild.sh` or `winBuild.bat` to set up the project and build it.

**MacOS**:
```sh
./macBuild.sh
```
**Windows**:
```sh
./winBuild.bat
```
## Running The Project
After building, the executable file should be located in the `build` directory with the name `GlorpEngine`.

### Aditional Notes
* For shader compilation, ensure `glslangValidator` is available in your system path.

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

## Credits
Based on Brendan Galea's [YouTube Series](https://www.youtube.com/@BrendanGalea)
## License
This project is licensed under the [MIT](https://choosealicense.com/licenses/mit/) license.
