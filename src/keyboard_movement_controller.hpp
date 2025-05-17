#pragma once

#include "glorp_game_object.hpp"

namespace Glorp {

class KeyboardMovementController {
    public:
        KeyboardMovementController(GlorpWindow &glorpWindow);
        
        void handleKeyboardMovement(float dt, GlorpGameObject &gameObject);
        void handleMouseInput(GlorpGameObject &gameObject);

    private:
        void updateMouseState();

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
        float m_yaw = 0.0f;
        float m_pitch = 0.0f;
    };
}
