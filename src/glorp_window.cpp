#include "glorp_window.hpp"

#include <stdexcept>
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

void GlorpWindow::InitWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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
}