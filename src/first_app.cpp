#include "first_app.hpp"

#include "glorp_cubemap.hpp"
#include "glorp_game_object.hpp"
#include "glorp_imgui.hpp"
#include "systems/cubemap_render_system.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"

#include "glorp_camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "glorp_buffer.hpp"

#include <GLFW/glfw3.h>
#include <chrono>
#include <cassert>
#include <thread>
#include <vulkan/vulkan_core.h>

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
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GlorpSwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
    
    loadGameObjects();

    texturePool = GlorpDescriptorPool::Builder(m_glorpDevice)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GlorpSwapChain::MAX_FRAMES_IN_FLIGHT * 5 * m_gameObjects.size())
        .build();
    glfwSetWindowUserPointer(m_glorpWindow.getGLFWwindow(), this);
    glfwSetFramebufferSizeCallback(m_glorpWindow.getGLFWwindow(), frameBufferResizeCallback);
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
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    GlorpCubeMap cubemap = GlorpCubeMap(m_glorpDevice, {
        std::string(RESOURCE_LOCATIONS) + "cubemap/skybox/right.jpg",
        std::string(RESOURCE_LOCATIONS) + "cubemap/skybox/left.jpg",
        std::string(RESOURCE_LOCATIONS) + "cubemap/skybox/bottom.jpg",
        std::string(RESOURCE_LOCATIONS) + "cubemap/skybox/top.jpg",
        std::string(RESOURCE_LOCATIONS) + "cubemap/skybox/front.jpg",
        std::string(RESOURCE_LOCATIONS) + "cubemap/skybox/back.jpg",
    });

    std::vector<VkDescriptorSet> globalDescriptorSets(GlorpSwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        VkDescriptorImageInfo skyboxInfo{
            .sampler = cubemap.getSampler(),
            .imageView = cubemap.getImageView(),
            .imageLayout = cubemap.getImageLayout()
        };
        GlorpDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .writeImage(1, &skyboxInfo)
            .build(globalDescriptorSets[i]);
    }

    auto textureSetLayout = GlorpDescriptorSetLayout::Builder(m_glorpDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Albedo
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Normal Map
        .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Emissive Map
        .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // AO Map
        .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // Metallic Roughness Map
        .build();

    for (auto &kv : m_gameObjects) {
        auto& obj = kv.second;
        if (obj.model == nullptr) continue;
        if (obj.material == nullptr) continue;

        // auto& material = obj.material;
        VkDescriptorImageInfo albedoImageInfo {};
        albedoImageInfo.sampler = obj.material->albedoTexture->getSampler();
        albedoImageInfo.imageView = obj.material->albedoTexture->getImageView();
        albedoImageInfo.imageLayout = obj.material->albedoTexture->getImageLayout();

        VkDescriptorImageInfo normalImageInfo {};
        normalImageInfo.sampler = obj.material->normalTexture->getSampler();
        normalImageInfo.imageView = obj.material->normalTexture->getImageView();
        normalImageInfo.imageLayout = obj.material->normalTexture->getImageLayout();

        VkDescriptorImageInfo emissiveImageInfo {};
        emissiveImageInfo.sampler = obj.material->emissiveTexture->getSampler();
        emissiveImageInfo.imageView = obj.material->emissiveTexture->getImageView();
        emissiveImageInfo.imageLayout = obj.material->emissiveTexture->getImageLayout();

        VkDescriptorImageInfo aoImageInfo {};
        aoImageInfo.sampler = obj.material->aoTexture->getSampler();
        aoImageInfo.imageView = obj.material->aoTexture->getImageView();
        aoImageInfo.imageLayout = obj.material->aoTexture->getImageLayout();

        VkDescriptorImageInfo metallicImageInfo {};
        metallicImageInfo.sampler = obj.material->metallicRoughnessTexture->getSampler();
        metallicImageInfo.imageView = obj.material->metallicRoughnessTexture->getImageView();
        metallicImageInfo.imageLayout = obj.material->metallicRoughnessTexture->getImageLayout();

        GlorpDescriptorWriter(*textureSetLayout, *texturePool)
            .writeImage(0, &albedoImageInfo)
            .writeImage(1, &normalImageInfo)
            .writeImage(2, &emissiveImageInfo)
            .writeImage(3, &aoImageInfo)
            .writeImage(4, &metallicImageInfo)
            .build(obj.descriptorSet);
    }


    SimpleRenderSystem simpleRenderSystem{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()};
    PointLightSystem pointLightSystem{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
    CubeMapRenderSystem cubemapRenderSystem{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
    GlorpImgui glorpImgui{m_glorpDevice, m_glorpRenderer.getSwapChainRenderPass(), m_glorpWindow};

    GlorpCamera camera{};

    auto viewerObject = GlorpGameObject::createGameObject();
    viewerObject.transform.translation.z = -2.5f;
    KeyboardMovementController cameraController(m_glorpWindow);
    
    std::thread renderThread([&] {
        auto currentTime = std::chrono::high_resolution_clock::now();
        while(!m_glorpWindow.shouldClose()) {

        // recreate swapchain while resizing window
        if(m_frameBufferResized) {
            int width = 0; int height = 0;
            glfwGetFramebufferSize(m_glorpWindow.getGLFWwindow(), &width, &height);
            if (width == 0 || height == 0) {
                glfwGetFramebufferSize(m_glorpWindow.getGLFWwindow(), &width, &height);
                glfwWaitEvents();
            }

            vkDeviceWaitIdle(m_glorpDevice.device());

            m_glorpRenderer.recreateSwapChain();
            float aspect = m_glorpRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

            m_frameBufferResized = false;
        }
        
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(frameTime, viewerObject);
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
                    glorpImgui.getRotationMultiplier(),
                    glorpImgui.useNormalMap,
                    glorpImgui.useAlbedoMap,
                    glorpImgui.useEmissiveMap,
                    glorpImgui.useAOMap,
                    glorpImgui.lightPosition
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
            cubemapRenderSystem.renderCubemap(frameInfo);
            pointLightSystem.render(frameInfo);

            glorpImgui.drawUI(frameInfo);

            m_glorpRenderer.endSwapChainRenderPass(commandBuffer);
            m_glorpRenderer.endFrame();
        }
        }
        vkDeviceWaitIdle(m_glorpDevice.device());
    });

    // Polling loop
    while(!m_glorpWindow.shouldClose()) {
        glfwPollEvents();
    }
    renderThread.join();
}

void FirstApp::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<FirstApp*>(window);
    app->m_frameBufferResized = true;
}

void FirstApp::loadGameObjects() {

    GlorpGameObject helmet = GlorpGameObject::createGameObjectFromAscii(m_glorpDevice, "models/DamagedHelmet/DamagedHelmet.gltf");
    helmet.transform.translation = {.0f, .0f, .0f};
    helmet.transform.scale = {1.f, 1.f, 1.f};
    m_gameObjects.emplace(helmet.getId(), std::move(helmet));

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
