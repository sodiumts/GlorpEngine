#include "first_app.hpp"

#include "simple_render_system.hpp"
#include "glorp_camera.hpp" 
#include "keyboard_movement_controller.hpp"

#include <stdexcept>
#include <array>
#include <chrono>

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
    GlorpCamera camera{};
    
    auto viewerObject = GlorpGameObject::createGameObject();
    KeyboardMovementController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();
    while(!m_glorpWindow.shouldClose()) {
        glfwPollEvents();
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(m_glorpWindow.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = m_glorpRenderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

        if (auto commandBuffer = m_glorpRenderer.beginFrame()) {
            m_glorpRenderer.beginSwapChainRenderPass(commandBuffer);
            SimpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects, camera);
            m_glorpRenderer.endSwapChainRenderPass(commandBuffer);
            m_glorpRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_glorpDevice.device());
}

void FirstApp::loadGameObjects() {
    std::shared_ptr<GlorpModel> glorpModel = GlorpModel::createModelFromFile(m_glorpDevice, "models/smooth_vase.obj");

    auto gameObject = GlorpGameObject::createGameObject();

    gameObject.model = glorpModel;
    gameObject.transform.translation = {.0f, .0f, 2.5f};
    gameObject.transform.scale = {.5f, .5f, .5f};
    m_gameObjects.push_back(std::move(gameObject));
}
}