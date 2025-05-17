#pragma once

#include "glorp_window.hpp"
#include "glorp_device.hpp"
#include "glorp_game_object.hpp"
#include "glorp_renderer.hpp"
#include "glorp_descriptors.hpp"
#include "glorp_texture.hpp"

#include <memory>
#include <atomic>

#ifndef RESOURCE_LOCATIONS
#define RESOURCE_LOCATIONS ""
#endif

namespace Glorp {
class FirstApp {
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 960;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();
    private:
        void loadGameObjects();
        void initImgui();
    private:
        GlorpWindow m_glorpWindow {WIDTH, HEIGHT, "Glorp Engine"};
        GlorpDevice m_glorpDevice {m_glorpWindow};
        GlorpRenderer m_glorpRenderer {m_glorpWindow, m_glorpDevice};


        id_t carRef;
        std::unique_ptr<GlorpDescriptorPool> globalPool {};
        std::unique_ptr<GlorpDescriptorPool> texturePool {};
        std::unique_ptr<GlorpDescriptorPool> cubemapPool {};
        std::shared_ptr<GlorpTexture> m_globalTexture;
        GlorpGameObject::Map m_gameObjects;
};

}
