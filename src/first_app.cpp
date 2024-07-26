#include "first_app.hpp"

#include "glorp_imgui.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"

#include "glorp_camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "glorp_buffer.hpp"
#include "glorp_texture.hpp"

#include <chrono>
#include <iostream>
#include <cassert>

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

    texturePool = GlorpDescriptorPool::Builder(m_glorpDevice)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GlorpSwapChain::MAX_FRAMES_IN_FLIGHT * m_gameObjects.size())
        .build();
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

    auto textureSetLayout = GlorpDescriptorSetLayout::Builder(m_glorpDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    for (auto &kv : m_gameObjects) {
        auto& obj = kv.second;
        if(obj.model == nullptr) continue;

        VkDescriptorImageInfo objImageInfo {};
        objImageInfo.sampler = obj.texture->getSampler();
        objImageInfo.imageView = obj.texture->getImageView();
        objImageInfo.imageLayout = obj.texture->getImageLayout();

        GlorpDescriptorWriter(*textureSetLayout, *texturePool)
            .writeImage(0, &objImageInfo)
            .build(obj.descriptorSet);
    }

    SimpleRenderSystem simpleRenderSystem{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()};
    PointLightSystem pointLightSystem{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
    GlorpImgui glorpImgui{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), m_glorpWindow};

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
                m_gameObjects,
                glorpImgui.getLightIntensity(),
                glorpImgui.getRotationMultiplier()
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

            glorpImgui.drawUI(frameInfo);

            m_glorpRenderer.endSwapChainRenderPass(commandBuffer);
            m_glorpRenderer.endFrame();
        }
    }
    vkDeviceWaitIdle(m_glorpDevice.device());
}

void FirstApp::loadGameObjects() {
    m_globalTexture = std::make_shared<Texture>(m_glorpDevice, "textures/missing_texture.png");

    // std::shared_ptr<GlorpModel> smoothVaseModel = GlorpModel::createModelFromFile(m_glorpDevice, std::string(MODELS_DIR) + "/smooth_vase.obj");
    // std::shared_ptr<GlorpModel> flatVaseModel = GlorpModel::createModelFromFile(m_glorpDevice, std::string(MODELS_DIR) + "/flat_vase.obj");
    std::shared_ptr<GlorpModel> vikingRoom = GlorpModel::createModelFromFile(m_glorpDevice, "models/viking_room.obj");
    auto vikingTexture = std::make_shared<Texture>(m_glorpDevice,"textures/viking_room.png");

    createGameObject(vikingRoom, vikingTexture, {.0f, .0f, .0f}, {1.f, 1.f, 1.f});
    // createGameObject(smoothVaseModel, nullptr, {1.5f, .5f, 0.f}, {3.f, 1.5f, 3.f});
    // createGameObject(flatVaseModel, nullptr, {.5f, .5f, 0.f}, {3.f, 1.5f, 3.f});

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
void FirstApp::createGameObject(std::shared_ptr<GlorpModel> model, std::shared_ptr<Texture> texture, glm::vec3 translation, glm::vec3 scale) {
    auto gameObj = GlorpGameObject::createGameObject();
    gameObj.model = model;
    gameObj.transform.translation = translation;
    gameObj.transform.scale = scale;

    gameObj.texture = texture ? texture : m_globalTexture;
    m_gameObjects.emplace(gameObj.getId(), std::move(gameObj));
}

}
