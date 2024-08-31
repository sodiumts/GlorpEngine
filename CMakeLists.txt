set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
cmake_minimum_required(VERSION 3.29.0)
project(GlorpEngine VERSION 0.1.0)

include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
if (NOT LOCAL_ENV)
    message(STATUS "Local .env.cmake not found, using default values")
else()
    message(STATUS "Local .env.cmake: ${LOCAL_ENV}")
endif()

option(BUILD_APP_BUNDLE "Build a macOS app bundle" OFF)
message(STATUS "Using CMake generator: ${CMAKE_GENERATOR}")

if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH)
        message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW True)
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER ${MINGW_PATH}/bin/g++.exe)
endif()

if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND True)
else()
    find_package(Vulkan REQUIRED)
    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using Vulkan library at: ${Vulkan_LIBRARIES}")
endif()

include(FetchContent)

if (DEFINED GLFW_PATH)
    message(STATUS "Using GLFW path specified in .env")
    set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
    if (MSVC)
        set(GLFW_LIB "${GLFW_PATH}/lib-vc2019")
    elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
        message(STATUS "Using MinGW")
        set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")
    endif()
else()
    find_package(glfw3 3.4 REQUIRED)
    get_target_property(GLFW_LIB_PATH glfw IMPORTED_LOCATION)
    if (NOT glfw3_FOUND)
        message(STATUS "GLFW not found, using FetchContent to download it")
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG        3.4
        )
        FetchContent_MakeAvailable(glfw)
        set(GLFW_LIB glfw)
    else()
        set(GLFW_LIB glfw3)
        message(STATUS "Found GLFW")
    endif()
endif()

if (NOT GLFW_LIB)
    message(FATAL_ERROR "Could not find GLFW library!")
else()
    message(STATUS "Using GLFW library at: ${GLFW_LIB}")
endif()

if (NOT TINYOBJ_PATH)
    message(STATUS "TINYOBJ_PATH not specified in .env.cmake, using external/tinyobjloader")
    set(TINYOBJ_PATH external/tinyobjloader)
endif()

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB IMGUI_SOURCES
    ${PROJECT_SOURCE_DIR}/external/imgui/*.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_vulkan.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
)



if (WIN32)
    add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

    message(STATUS "Creating build for Windows")

    if (USE_MINGW)
        target_include_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/include)
        target_link_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/lib)
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/external
        ${PROJECT_SOURCE_DIR}/external/imgui
        ${PROJECT_SOURCE_DIR}/external/imgui/backends
        ${Vulkan_INCLUDE_DIRS}
        ${TINYOBJ_PATH}
        ${GLFW_INCLUDE_DIRS}
    )

    target_link_directories(${PROJECT_NAME} PUBLIC
        ${Vulkan_LIBRARIES}
        ${GLFW_LIB}
    )

    target_link_libraries(${PROJECT_NAME} glfw3 vulkan-1)

elseif(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 11)

    if(BUILD_APP_BUNDLE)
        message(STATUS "Creating macOS app bundle")

        if (NOT VULKAN_MAC_LOCATION)
            message(FATAL_ERROR "VULKAN_MAC_LOCATION not set in .env.cmake")
        endif()

        if (NOT GLFW_MAC_LOCATION)
            message(FATAL_ERROR "GLFW_MAC_LOCATION not set in .env.cmake")
        endif()
        # set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment target")

        set(RESOURCE_LOCATIONS "Resources/")
        set(IMAGE_FILE ${PROJECT_SOURCE_DIR}/assets/glorp.icns)
        set(INFO_PLIST ${PROJECT_SOURCE_DIR}/assets/Info.plist)
        set(VULKAN_ICD ${PROJECT_SOURCE_DIR}/assets/MoltenVK_icd.json)

        file(GLOB_RECURSE SHADER_FILES "${PROJECT_SOURCE_DIR}/shaders/*.spv")
        file(GLOB_RECURSE TEXTURE_FILES "${PROJECT_SOURCE_DIR}/textures/*.png")
        file(GLOB_RECURSE MODEL_FILES "${PROJECT_SOURCE_DIR}/models/*.obj")

        foreach(file ${SHADER_FILES})
            set_source_files_properties(${file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/shaders")
        endforeach()
        foreach(file ${TEXTURE_FILES})
            set_source_files_properties(${file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/textures")
        endforeach()
        foreach(file ${MODEL_FILES})
            set_source_files_properties(${file} PROPERTIES HEADER_FILE_ONLY TRUE MACOSX_PACKAGE_LOCATION "Resources/models")
        endforeach()
        set_source_files_properties(${IMAGE_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${IMGUI_SOURCES} ${SHADER_FILES} ${TEXTURE_FILES} ${MODEL_FILES} "${IMAGE_FILE}")
        target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

        set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "${CMAKE_PROJECT_NAME}"
            OSX_ARCHITECTURES "arm64"
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.sodiumTsss.glorpEngine"
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
            MACOSX_BUNDLE_ICON_FILE glorp.icns
            MACOSX_BUNDLE_INFO_PLIST "${INFO_PLIST}"
        )

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Frameworks"

            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/vulkan/icd.d/"

            COMMAND ${CMAKE_COMMAND} -E copy
                "${GLFW_MAC_LOCATION}/lib/libglfw.3.dylib"
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Frameworks/"

            COMMAND ${CMAKE_COMMAND} -E copy
                "${VULKAN_ICD}"
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources/vulkan/icd.d/"

            COMMAND ${CMAKE_COMMAND} -E copy
                "${VULKAN_MAC_LOCATION}/macos/lib/libvulkan.1.dylib"
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Frameworks/"

            COMMAND ${CMAKE_COMMAND} -E copy
                "${VULKAN_MAC_LOCATION}/macos/lib/libMoltenVK.dylib"
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Frameworks/"

            COMMAND install_name_tool -add_rpath @executable_path/../Frameworks
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/MacOS/${PROJECT_NAME}"

            COMMAND install_name_tool -change ${GLFW_MAC_LOCATION}/lib/libglfw.3.dylib @rpath/libglfw.3.dylib
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/MacOS/${PROJECT_NAME}"

            COMMAND install_name_tool -change ${VULKAN_MAC_LOCATION}/macos/lib/libvulkan.1.dylib @rpath/libvulkan.1.dylib
                "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/MacOS/${PROJECT_NAME}"
        )
    else()
        message(STATUS "Creating macOS executable")
        add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})
        target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/external
        ${PROJECT_SOURCE_DIR}/external/imgui
        ${PROJECT_SOURCE_DIR}/external/imgui/backends
        ${TINYOBJ_PATH}
        ${PROJECT_SOURCE_DIR}/src
    )

    target_link_libraries(${PROJECT_NAME} glfw Vulkan::Vulkan)
    target_compile_definitions(GlorpEngine PRIVATE APPLE)

elseif (UNIX)
    message(STATUS "Creating build for UNIX")

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${TINYOBJ_PATH}
    )

    target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})
endif()


target_compile_definitions(GlorpEngine PRIVATE SHADERS_DIR="${SHADERS_DIR}" MODELS_DIR="${MODELS_DIR}")

find_program(GLSL_VALIDATOR glslangValidator HINTS
    ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
    /usr/bin
    /usr/local/bin
    ${VULKAN_SDK_PATH}/Bin
    ${VULKAN_SDK_PATH}/Bin32
    $ENV{VULKAN_SDK}/Bin/
    $ENV{VULKAN_SDK}/Bin32/
)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
