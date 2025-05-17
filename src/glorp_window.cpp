#include "glorp_window.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_mouse.h>
#include <stdexcept>
#include <iostream>
namespace Glorp {

GlorpWindow::GlorpWindow(int width, int height, const std::string windowName)  :m_width(width), m_height(height), m_windowName(windowName)
{
    InitWindow();
}

GlorpWindow::~GlorpWindow()
{
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void GlorpWindow::InitWindow() {

    if(!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("Failed to init SDL");
    }

    SDL_WindowFlags wFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    m_window = SDL_CreateWindow("Glorp Engine", m_width, m_height, wFlags);
    SDL_SetWindowRelativeMouseMode(m_window, true);
}

void GlorpWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR * surface) {
    if(SDL_Vulkan_CreateSurface(m_window, instance, nullptr, surface) == 0) {
        throw std::runtime_error("Failed to create surface");
    }
}

void GlorpWindow::windowResize() {
    m_frameBufferResized = true;
    SDL_GetWindowSize(m_window, &m_width, &m_height);
}
}
