#include "glorp_window.hpp"

#include <stdexcept>
#include <iostream>
namespace Glorp {

GlorpWindow::GlorpWindow(int width, int height, const std::string windowName)  :m_width(width), m_height(height), m_windowName(windowName)
{
    InitWindow();
}

GlorpWindow::~GlorpWindow()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}
void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}
void GlorpWindow::InitWindow() {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    // if(fullscreenBorderless) {
    //     GLFWmonitor* primary = glfwGetPrimaryMonitor();
    //     const GLFWvidmode* mode = glfwGetVideoMode(primary);
    
    //     glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    //     glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    //     glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    //     glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    //     m_window = glfwCreateWindow(mode->width, mode->height, m_windowName.c_str(), primary, NULL);
    // } else {
        
    // }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallback);
}
void GlorpWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR * surface) {
    if(glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a window surface");
    }
}
void GlorpWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto glorpWindow = reinterpret_cast<GlorpWindow *>(glfwGetWindowUserPointer(window));
    glorpWindow->m_frameBufferResized = true;
    glorpWindow->m_width = width;
    glorpWindow->m_height = height;
}

void GlorpWindow::toggleFullscreen() {
    if (fullscreen) {
        glfwSetWindowMonitor(m_window, NULL, 0, 0, m_width, m_height, GLFW_DONT_CARE);
        fullscreen = false;
        glfwSetWindowSize(m_window, m_windowedWidth, m_windowedHeight);
        glfwSetWindowPos(m_window, m_windowedPositionX, m_windowedPositionY);
    } else {
        glfwGetWindowSize(m_window, &m_windowedWidth, &m_windowedHeight);
        glfwGetWindowPos(m_window, &m_windowedPositionX, &m_windowedPositionY);

        auto monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        fullscreen = true;
    }
}
}