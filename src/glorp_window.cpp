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

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallback);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
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
