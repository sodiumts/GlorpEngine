#include "keyboard_movement_controller.hpp"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Glorp {

KeyboardMovementController::KeyboardMovementController(GlorpWindow &glorpWindow)
: m_glorpWindow(glorpWindow)
{ }

void KeyboardMovementController::updateMouseState(GLFWwindow* window) {
    if (m_mouseEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        m_lastX = width / 2.0;
        m_lastY = height / 2.0;
        m_firstMouse = true;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        }
    }
}

void KeyboardMovementController::handleMouseInput(GLFWwindow* window, GlorpGameObject &gameObject) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    float deltaX = static_cast<float>(xpos - m_lastX);
    float deltaY = static_cast<float>(m_lastY - ypos);

    m_lastX = xpos;
    m_lastY = ypos;

    gameObject.transform.rotation.y += deltaX * m_mouseSensitivity;
    gameObject.transform.rotation.x += deltaY * m_mouseSensitivity;

    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
}

void KeyboardMovementController::handleKeyboardMovement(GLFWwindow* window, float dt, GlorpGameObject &gameObject) {
    float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
    const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
    const glm::vec3 upDir{0.f, -1.f, 0.f};

    glm::vec3 moveDir{0.f};
    if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
        gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
    }
}

void KeyboardMovementController::moveInPlaneXZ(float dt, GlorpGameObject &gameObject) {
    GLFWwindow *window = m_glorpWindow.getGLFWwindow();

    bool currentAltState = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS;
    if (currentAltState && !m_altKeyPressed) {
        m_mouseEnabled = !m_mouseEnabled;
        updateMouseState(window);
    }
    m_altKeyPressed = currentAltState;

    if (m_mouseEnabled) {
        handleMouseInput(window, gameObject);
    }

    handleKeyboardMovement(window, dt, gameObject);
}

}
