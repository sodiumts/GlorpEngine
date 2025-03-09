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
    glm::mat4 inverseProjection{1.f};
    glm::mat4 view{1.f};
    glm::mat4 inverseView{1.f};
    glm::vec4 ambientLightColor {1.f, 1.f, 1.f, 0.02f};
    PointLight pointLights[MAX_LIGHTS];
    int numLights;
    glm::vec2 resolution;
};

struct FrameInfo {
    int frameIndex;
    float frameTime;
    VkCommandBuffer commandBuffer;
    GlorpCamera &camera;
    VkDescriptorSet globalDescriptorSet;
    VkDescriptorSet skyDescriptorSet;
    GlorpGameObject::Map &gameObjects;
    float lightIntensity;
    float lightRotationMultiplier;
    glm::vec3 sunDirection;

    bool useNormalMap{true};
    bool useAlbedoMap{true};
    bool useEmissiveMap{true};
    bool useAOMap{true};

    float lightVerticalPosition;
};

}
