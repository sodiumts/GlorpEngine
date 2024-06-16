#include "first_app.hpp"

#include "simple_render_system.hpp"

#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Glorp {
FirstApp::FirstApp() {
    loadGameObjects();
}
FirstApp::~FirstApp() {}


void FirstApp::run() {
    SimpleRenderSystem SimpleRenderSystem{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass()};

    while(!m_glorpWindow.shouldClose()) {
        glfwPollEvents();
        if (auto commandBuffer = m_glorpRenderer.beginFrame()) {
            m_glorpRenderer.beginSwapChainRenderPass(commandBuffer);
            SimpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects);
            m_glorpRenderer.endSwapChainRenderPass(commandBuffer);
            m_glorpRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_glorpDevice.device());
}


void FirstApp::loadGameObjects() {
    std::vector<GlorpModel::Vertex> vertices {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    auto glorpModel = std::make_shared<GlorpModel>(m_glorpDevice, vertices);

    auto triangle = GlorpGameObject::createGameObject();
    triangle.model = glorpModel;
    triangle.color = {.1f, .8f, .1f};
    triangle.transform2d.translation.x = .2f;
    triangle.transform2d.scale = {2.f, .5f};
    triangle.transform2d.rotation = .25f * glm::two_pi<float>();

    m_gameObjects.push_back(std::move(triangle));
}
}