#include "first_app.hpp"

#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"

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

FirstApp::FirstApp() {
    globalPool = GlorpDescriptorPool::Builder(m_glorpDevice)
        .setMaxSets(GlorpSwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GlorpSwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();

    loadGameObjects();
}
FirstApp::~FirstApp() {}


void FirstApp::run() {
    std::vector<std::unique_ptr<GlorpBuffer>> uboBuffers(GlorpSwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<GlorpBuffer>(
            m_glorpDevice,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffers[i]->map();
    }


    auto globalSetLayout = GlorpDescriptorSetLayout::Builder(m_glorpDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(GlorpSwapChain::MAX_FRAMES_IN_FLIGHT);

    for(int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        GlorpDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    SimpleRenderSystem simpleRenderSystem{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
    PointLightSystem pointLightSystem{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

    GlorpCamera camera{};
    
    auto viewerObject = GlorpGameObject::createGameObject();
    viewerObject.transform.translation.z = -2.5f;
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
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);


        if (auto commandBuffer = m_glorpRenderer.beginFrame()) {
            int frameIndex = m_glorpRenderer.getFrameIndex();

            FrameInfo frameInfo {
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                m_gameObjects
            };
            //update
            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            pointLightSystem.update(frameInfo, ubo);
            
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // render
            m_glorpRenderer.beginSwapChainRenderPass(commandBuffer);
            
            
            simpleRenderSystem.renderGameObjects(frameInfo);
            pointLightSystem.render(frameInfo);

            m_glorpRenderer.endSwapChainRenderPass(commandBuffer);
            m_glorpRenderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m_glorpDevice.device());
}

void FirstApp::loadGameObjects() {
    std::shared_ptr<GlorpModel> smoothVaseModel = GlorpModel::createModelFromFile(m_glorpDevice, std::string(MODELS_DIR) + "/smooth_vase.obj");

    auto smoothVaseObject = GlorpGameObject::createGameObject();

    smoothVaseObject.model = smoothVaseModel;
    smoothVaseObject.transform.translation = {-.5f, .5f, 0.f};
    smoothVaseObject.transform.scale = {3.f, 1.5f, 3.f};
    m_gameObjects.emplace(smoothVaseObject.getId(), std::move(smoothVaseObject));

    std::shared_ptr<GlorpModel> flatVaseModel = GlorpModel::createModelFromFile(m_glorpDevice, std::string(MODELS_DIR) + "/flat_vase.obj");
    auto flatVaseObject = GlorpGameObject::createGameObject();

    flatVaseObject.model = flatVaseModel;
    flatVaseObject.transform.translation = {.5f, .5f, 0.f};
    flatVaseObject.transform.scale = {3.f, 1.5f, 3.f};
    m_gameObjects.emplace(flatVaseObject.getId(), std::move(flatVaseObject));

    std::shared_ptr<GlorpModel> floorModel = GlorpModel::createModelFromFile(m_glorpDevice, std::string(MODELS_DIR) + "/quad.obj");
    auto floorObject = GlorpGameObject::createGameObject();

    floorObject.model = floorModel;
    floorObject.transform.translation = {.0f, .5f, .0f};
    floorObject.transform.scale = {3.f, 1.f, 3.f};
    m_gameObjects.emplace(floorObject.getId(), std::move(floorObject));
 

    std::vector<glm::vec3> lightColors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f} 
    };

    for (int i = 0; i < lightColors.size(); i++) {
        auto pl = GlorpGameObject::makePointLight(0.5f);
        pl.color = lightColors[i];
        auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), {0.f, -1.f, 0.f});
        pl.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
        m_gameObjects.emplace(pl.getId(), std::move(pl));
    }
}

}