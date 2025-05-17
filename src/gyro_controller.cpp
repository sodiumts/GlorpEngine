#include "gyro_controller.hpp"
#include "glorp_game_object.hpp"
#include <glm/fwd.hpp>

namespace Glorp {

GyroController::GyroController(GlorpWindow &window): m_window(window) {};

void GyroController::handleGyroMovement(float dt, GlorpGameObject &gameObject) {
    SDL_UpdateGamepads();
    float gamepadValues[3];
    if (!SDL_GetGamepadSensorData(m_window.getGamepad(), SDL_SENSOR_GYRO, gamepadValues, 3)) {
        throw std::runtime_error("Failed to get sensor data");
    }
    // Reset on misc button press
    if(SDL_GetGamepadButton(m_window.getGamepad(), SDL_GAMEPAD_BUTTON_MISC1)) {
        gameObject.transform.rotation = glm::quat{};
    }

    glm::vec3 angularVelocity(
        gamepadValues[1],  // Pitch (around X)
        gamepadValues[2],  // Yaw (around Y)
        gamepadValues[0]   // Roll (around Z)
    );

    float angle = glm::length(angularVelocity) * dt;
    if (angle > 0.0f) {
        glm::vec3 rotationAxis = glm::normalize(angularVelocity);
        
        glm::quat deltaRot = glm::angleAxis(angle, rotationAxis);
        gameObject.transform.rotation = gameObject.transform.rotation * deltaRot;
        
        gameObject.transform.rotation = glm::normalize(gameObject.transform.rotation);
    }
}

}
