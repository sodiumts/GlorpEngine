#pragma once

#include "glorp_camera.hpp"
#include "glorp_game_object.hpp"

#include <vulkan/vulkan.h>

namespace Glorp {

#define MAX_LIGHTS 10
    
struct PointLight {
    glm::vec4 position{};
    glm::vec4 color{};
};

struct GlobalUbo {
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec4 ambientLightColor {1.f, 1.f, 1.f, 0.02f};
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
};

struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    GlorpCamera &camera;
    VkDescriptorSet globalDescriptorSet;
    GlorpGameObject::Map &gameObjects;
};

}