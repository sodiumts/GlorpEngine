# Glorp Engine
Glorp engine is a Vulkan / GLFW based rendering engine for MacOS with ARM processors and Windows, written in C++
<img width="1363" alt="Screenshot 2024-09-01 at 02 48 23" src="https://github.com/user-attachments/assets/900ae3fe-4629-4757-870d-b5e188aa71d6">


*Model credits:*  
Source : https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main/Models/DamagedHelmet  
&copy; 2018, ctxwing. [CC BY 4.0 International](https://creativecommons.org/licenses/by/4.0/legalcode)

 - ctxwing for Rebuild and conversion to glTF

&copy; 2016, theblueturtle_. [CC BY-NC 4.0 International](https://creativecommons.org/licenses/by-nc/4.0/legalcode)

 - theblueturtle_ for Earlier version of model

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
* Everything else is statically compiled from the `external/` directory.
## Environment Setup
### .env.cmake
Create a `.env.cmake` file in the project root and specify the necessary dependency paths:
Example file:
```cmake
# .env.cmake
set(MINGW_PATH "C:/path/to/mingw")
set(VULKAN_SDK_PATH "C:/path/to/vulkan")
set(GLFW_PATH "C:/path/to/glfw")

# MacOS only (For app bundle creation)
set(VULKAN_MAC_LOCATION username/VulkanSDK/1.3.283.0)
set(GLFW_MAC_LOCATION username/Developer/libs/glfw-3.4)
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
Build and run unix-like binary:
```sh
./macBuild.sh
```
Build a macOS app bundle (must set ``VULKAN_MAC_LOCATION`` and ``GLFW_MAC_LOCATION`` to the appropriate locations in .env.cmake):
```sh
./macBuild.sh -r
```
*Don't forget to sign your app bundles for distribution!*

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
