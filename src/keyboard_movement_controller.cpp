#include "keyboard_movement_controller.hpp"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Glorp {

KeyboardMovementController::KeyboardMovementController(GlorpWindow &glorpWindow)
: m_glorpWindow(glorpWindow)
{ }

void KeyboardMovementController::updateMouseState() {
    if (m_mouseEnabled) {
        SDL_SetWindowRelativeMouseMode(m_glorpWindow.getSDLWindow(), true);
        SDL_GetRelativeMouseState(nullptr, nullptr);        

        m_firstMouse = true;
    } else {
        SDL_SetWindowRelativeMouseMode(m_glorpWindow.getSDLWindow(), false); 
    }
}

void KeyboardMovementController::handleMouseInput(GlorpGameObject &gameObject) {
    float deltaX, deltaY;
    SDL_GetRelativeMouseState(&deltaX, &deltaY);

    gameObject.transform.rotation.y += deltaX * m_mouseSensitivity;
    gameObject.transform.rotation.x -= deltaY * m_mouseSensitivity;

    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
}

void KeyboardMovementController::handleKeyboardMovement(float dt, GlorpGameObject &gameObject) {
    float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, -1.f, 0.f};
    
    glm::vec3 moveDir{0.f};
    int numkeys;
    const bool *keyStates = SDL_GetKeyboardState(&numkeys);
    if(keyStates[SDL_SCANCODE_W]) moveDir += forwardDir;
    if(keyStates[SDL_SCANCODE_S]) moveDir -= forwardDir;
    if(keyStates[SDL_SCANCODE_D]) moveDir += rightDir;
    if(keyStates[SDL_SCANCODE_A]) moveDir -= rightDir;
    if(keyStates[SDL_SCANCODE_SPACE]) moveDir += upDir;
    if(keyStates[SDL_SCANCODE_LSHIFT]) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
    }

    if(keyStates[SDL_SCANCODE_LALT] && !m_altKeyPressed) {
        m_mouseEnabled = !m_mouseEnabled;
        updateMouseState();
    }
    m_altKeyPressed = keyStates[SDL_SCANCODE_LALT];

    if(m_mouseEnabled)
        handleMouseInput(gameObject);
}
}
