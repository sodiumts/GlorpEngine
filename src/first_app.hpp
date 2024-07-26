#pragma once

#include "glorp_window.hpp"
#include "glorp_device.hpp"
#include "glorp_game_object.hpp"
#include "glorp_renderer.hpp"
#include "glorp_descriptors.hpp"
#include "glorp_texture.hpp"

#include <memory>

#ifndef RESOURCE_LOCATIONS
#define RESOURCE_LOCATIONS ""
#endif

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
        void initImgui();
        void createGameObject(std::shared_ptr<GlorpModel> model, std::shared_ptr<Texture> texture, glm::vec3 translation, glm::vec3 scale);
    private:
        GlorpWindow m_glorpWindow {WIDTH, HEIGHT, "Glorp Engine"};
        GlorpDevice m_glorpDevice {m_glorpWindow};
        GlorpRenderer m_glorpRenderer {m_glorpWindow, m_glorpDevice};

        std::unique_ptr<GlorpDescriptorPool> globalPool {};
        std::unique_ptr<GlorpDescriptorPool> texturePool {};
        std::shared_ptr<Texture> m_globalTexture;
        GlorpGameObject::Map m_gameObjects;
};

}
