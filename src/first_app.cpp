#include "first_app.hpp"

#include "simple_render_system.hpp"
#include "glorp_camera.hpp" 
#include "keyboard_movement_controller.hpp"
#include "glorp_buffer.hpp"

#include <stdexcept>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#ifndef MODELS_DIR
#define MODELS_DIR
#endif

namespace Glorp {

struct GlobalUbo {
    glm::mat4 projectionView{1.f};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

FirstApp::FirstApp() {
    loadGameObjects();
}
FirstApp::~FirstApp() {}


void FirstApp::run() {

    GlorpBuffer globalUboBuffer {
        m_glorpDevice,
        sizeof(GlobalUbo),
        GlorpSwapChain::MAX_FRAMES_IN_FLIGHT,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        m_glorpDevice.properties.limits.minUniformBufferOffsetAlignment
    };

    globalUboBuffer.map();

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
            int frameIndex = m_glorpRenderer.getFrameIndex();

            FrameInfo frameInfo {
                frameIndex,
                frameTime,
                commandBuffer,
                camera
            };
            //update
            GlobalUbo ubo{};
            ubo.projectionView = camera.getProjection() * camera.getView();
            globalUboBuffer.writeToIndex(&ubo ,frameIndex);
            globalUboBuffer.flushIndex(frameIndex);

            // render
            m_glorpRenderer.beginSwapChainRenderPass(commandBuffer);
            SimpleRenderSystem.renderGameObjects(frameInfo, m_gameObjects);
            m_glorpRenderer.endSwapChainRenderPass(commandBuffer);
            m_glorpRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_glorpDevice.device());
}

void FirstApp::loadGameObjects() {
    std::shared_ptr<GlorpModel> catModel = GlorpModel::createModelFromFile(m_glorpDevice, std::string(MODELS_DIR) + "/smooth_vase.obj");

    auto catObject = GlorpGameObject::createGameObject();

    catObject.model = catModel;
    catObject.transform.translation = {.0f, 0.f, 2.5f};
    catObject.transform.scale = {.5f, .5f, .5f};
    catObject.transform.rotation = {.5f, .5f, .7f};
    m_gameObjects.push_back(std::move(catObject));

    std::shared_ptr<GlorpModel> floorModel = GlorpModel::createModelFromFile(m_glorpDevice, std::string(MODELS_DIR) + "/flat_vase.obj");

    auto floorObject = GlorpGameObject::createGameObject();

    floorObject.model = floorModel;
    floorObject.transform.translation = {.0f, .6f, 2.f};
    floorObject.transform.scale = {.5f, .5f, .5f};
    floorObject.transform.rotation = {.0f, .0f, .0f};
    m_gameObjects.push_back(std::move(floorObject));

}

}