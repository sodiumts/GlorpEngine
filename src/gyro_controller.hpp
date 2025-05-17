#pragma once

#include "glorp_game_object.hpp"
#include "glorp_window.hpp"
namespace Glorp {

class GyroController {
    public:
        GyroController(GlorpWindow& window);
        
        void handleGyroMovement(float dt, GlorpGameObject &gameObject);


    private:
        GlorpWindow &m_window; 

};
}
