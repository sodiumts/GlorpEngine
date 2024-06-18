#pragma once 

#include "glorp_window.hpp"
#include "glorp_device.hpp"
#include "glorp_model.hpp"
#include "glorp_game_object.hpp"
#include "glorp_renderer.hpp"

#include <memory>
#include <vector>

namespace Glorp {
class FirstApp {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp &operator=(const FirstApp &) = delete;
        
        void run();
    private:
        void loadGameObjects();
    private:
        GlorpWindow m_glorpWindow {WIDTH, HEIGHT, "Glorp Engine"};
        GlorpDevice m_glorpDevice {m_glorpWindow};
        GlorpRenderer m_glorpRenderer {m_glorpWindow, m_glorpDevice};

        std::vector<GlorpGameObject> m_gameObjects;
};

}