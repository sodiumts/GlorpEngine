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

void KeyboardMovementController::handleMouseInput(GlorpGameObject &cameraObject) {
    float deltaX, deltaY;
    SDL_GetRelativeMouseState(&deltaX, &deltaY);

    m_yaw += -deltaX * m_mouseSensitivity;
    m_pitch += deltaY * m_mouseSensitivity;

    const float maxPitch = glm::radians(89.0f);
    m_pitch = glm::clamp(m_pitch, -maxPitch, +maxPitch);

    glm::quat qYaw   = glm::angleAxis(m_yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat qPitch = glm::angleAxis(m_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    cameraObject.transform.rotation = glm::normalize(qYaw * qPitch);
}
void KeyboardMovementController::handleKeyboardMovement(float dt, GlorpGameObject &gameObject) {
    glm::vec3 forwardRaw = gameObject.transform.rotation * glm::vec3(0,0,-1);
    glm::vec3 rightDir = gameObject.transform.rotation * glm::vec3(1,0,0);
    const glm::vec3 upDir{0.f, -1.f, 0.f};
    glm::vec3 forwardDir = forwardRaw - glm::dot(forwardRaw, upDir) * upDir;
    forwardDir = glm::normalize(forwardDir);
    
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
