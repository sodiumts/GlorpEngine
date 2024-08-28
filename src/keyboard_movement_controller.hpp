#pragma once

#include "glorp_game_object.hpp"

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
        };

        void moveInPlaneXZ(float dt, GlorpGameObject &gameObject);

    private:
        KeyMappings keys{};
        float moveSpeed{3.f};
        float turnSpeed{1.5f};
        
        GlorpWindow &m_glorpWindow;
    };
}
