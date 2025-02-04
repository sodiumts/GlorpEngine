#pragma once

#include "glorp_game_object.hpp"
#include <GLFW/glfw3.h>

namespace Glorp {

class KeyboardMovementController {
    public:
        KeyboardMovementController(GlorpWindow &glorpWindow);
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_SPACE;
            int moveDown = GLFW_KEY_LEFT_SHIFT;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
            int toggleFullscreen = GLFW_KEY_F11;
            int toggleCursor = GLFW_KEY_LEFT_ALT;
        };

        void moveInPlaneXZ(float dt, GlorpGameObject &gameObject);

    private:
        void handleMouseInput(GLFWwindow* window, GlorpGameObject &gameObject);
        void updateMouseState(GLFWwindow* window);
        void handleKeyboardMovement(GLFWwindow* window, float dt, GlorpGameObject &gameObject);

        KeyMappings keys{};
        float moveSpeed{3.f};
        float turnSpeed{1.5f};
        bool m_firstMouse = true;
        bool m_altKeyPressed = false;
        double m_lastX = 0.0;
        double m_lastY = 0.0;
        float m_mouseSensitivity = 0.002f;
        bool m_inputInitialized = false;
        bool m_mouseEnabled = true;
        bool m_cursorPending = false;
        bool toggleFullscreenPending = false;
        GlorpWindow &m_glorpWindow;
    };
}
