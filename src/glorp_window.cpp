#include "glorp_window.hpp"
#include "SDL3/SDL_gamepad.h"
#include "SDL3/SDL_hints.h"
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

void GlorpWindow::setupGamepad(SDL_GamepadDeviceEvent event) {
    if (m_gamepad != NULL)
        return;

    auto gamepadID = event.which;
    m_gamepad = SDL_OpenGamepad(gamepadID);
    if (m_gamepad == NULL)
        return;
    std::cout << "Gamepad connected" << std::endl;
    
    if(!SDL_SetGamepadSensorEnabled(m_gamepad, SDL_SENSOR_GYRO, true)) {
        std::cout << "Gamepad does not support gyro" << std::endl;
        m_gamepadSupportsGyro = false;
        return;
    }
    m_gamepadSupportsGyro = true; 
}

void GlorpWindow::destroyGamepad(SDL_GamepadDeviceEvent event) {
    if(SDL_GetGamepadID(m_gamepad) != event.which)
        return;

    SDL_CloseGamepad(m_gamepad);
    std::cout << "Gamepad disconnected" << std::endl;
    m_gamepad = NULL;
    m_gamepadSupportsGyro = false;
}

void GlorpWindow::InitWindow() {
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_SENSOR)) {
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
