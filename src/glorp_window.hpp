#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace Glorp {

class GlorpWindow {
    public:
        GlorpWindow(int width, int height, const std::string windowName);
        ~GlorpWindow();
        
        GlorpWindow(const GlorpWindow&) = delete;
        GlorpWindow &operator=(const GlorpWindow &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(m_window); };
        void createWindowSurface(VkInstance instance, VkSurfaceKHR * surface);
        VkExtent2D getExtent() { return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}; }
        bool wasWindowResized() { return m_frameBufferResized; }
        void resetWindowResizedFlag() { m_frameBufferResized = false; }
        GLFWwindow* getGLFWwindow() const { return m_window; }
    private:
        void InitWindow();
        static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);
    
    private:
        int m_height;
        int m_width;
        bool m_frameBufferResized = false;

        const std::string m_windowName;
        GLFWwindow *m_window;
};
}