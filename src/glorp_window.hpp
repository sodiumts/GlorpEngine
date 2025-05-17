#pragma once

#include "SDL3/SDL_events.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <string>
#include <tuple>
#include <vulkan/vulkan_core.h>

namespace Glorp {

class GlorpWindow {
    public:
        GlorpWindow(int width, int height, const std::string windowName);
        ~GlorpWindow();
        
        GlorpWindow(const GlorpWindow&) = delete;
        GlorpWindow &operator=(const GlorpWindow &) = delete;

        void createWindowSurface(VkInstance instance, VkSurfaceKHR * surface);
        VkExtent2D getExtent() { return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}; }
        void windowResize();
        bool wasWindowResized() { return m_frameBufferResized; }
        void resetWindowResizedFlag() { m_frameBufferResized = false; }
        SDL_Window* getSDLWindow() const { return m_window; }
        std::tuple<int, int> getWidthHeight() { return std::make_tuple(m_width, m_height); }
        void setShouldClose(bool val) { m_shouldClose = val; }
        bool shouldClose() { return m_shouldClose; }
        SDL_Gamepad* getGamepad(){ return m_gamepad; }
        void setupGamepad(SDL_GamepadDeviceEvent event);
        void destroyGamepad(SDL_GamepadDeviceEvent event);
        bool getGyroSupport() { return m_gamepadSupportsGyro; }
    private:
        void InitWindow();
        static void frameBufferResizeCallback(SDL_Window *window, int width, int height);
    
    private:
        int m_height;
        int m_width;

        int m_windowedHeight;
        int m_windowedWidth;
        int m_windowedPositionX;
    int m_windowedPositionY;
        
        bool m_shouldClose = false;

        bool m_frameBufferResized = false;

        SDL_Gamepad* m_gamepad = NULL;
        bool m_gamepadSupportsGyro = false;
        bool fullscreen = false;

        const std::string m_windowName;
        SDL_Window* m_window;
};
}
