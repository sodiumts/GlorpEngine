#include "point_light_system.hpp"

#include <stdexcept>
#include <map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Glorp {

struct PointLightPushConstants {
    glm::vec4 position{};
    glm::vec4 color{};
    float radius;
};

PointLightSystem::PointLightSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): m_glorpDevice{device} {
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}
PointLightSystem::~PointLightSystem() {
    vkDestroyPipelineLayout(m_glorpDevice.device(), m_pipelineLayout, nullptr);
}


void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {

    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PointLightPushConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};


    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if(vkCreatePipelineLayout(m_glorpDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Could not create pipeline layout");
    }
}

void PointLightSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo) {
    int lightIndex = 0;

    auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime * frameInfo.lightRotationMultiplier, {0.f, -1.f, 0.f});

    for (auto& kv: frameInfo.gameObjects) {
        auto& obj = kv.second;
        if(obj.pointLight == nullptr) continue;
        assert(lightIndex < MAX_LIGHTS && "The maximum amount of lights has been exceeded");

        obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));

        obj.transform.translation.y = frameInfo.lightVerticalPosition;

        ubo.pointLights[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
        ubo.pointLights[lightIndex].color = glm::vec4(obj.color, frameInfo.lightIntensity);

        lightIndex++;
    }
    ubo.numLights = lightIndex;
}

void PointLightSystem::createPipeline(VkRenderPass renderPass) {
    assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};

    GlorpPipeline::defaultPipelineConfigInfo(pipelineConfig);
    GlorpPipeline::enableAlphaBlending(pipelineConfig);
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_glorpPipeline = std::make_unique<GlorpPipeline>(
        m_glorpDevice,
        std::string(RESOURCE_LOCATIONS) + "shaders/point_light.vert.spv",
        std::string(RESOURCE_LOCATIONS) + "shaders/point_light.frag.spv",
        pipelineConfig
    );
}
void PointLightSystem::render(FrameInfo &frameInfo) {

    std::map<float, GlorpGameObject::id_t> sorted;

    for (auto& kv: frameInfo.gameObjects) {
        auto& obj = kv.second;
        if(obj.pointLight == nullptr) continue;

        auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
        float distSquared = glm::dot(offset, offset);
        sorted[distSquared] = obj.getId();
    }

    m_glorpPipeline->bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        0, 1,
        &frameInfo.globalDescriptorSet,
        0, nullptr
    );



    for (auto it = sorted.rbegin(); it != sorted.rend(); it++) {
        auto& obj = frameInfo.gameObjects.at(it->second);

        PointLightPushConstants push{};
        push.position = glm::vec4(obj.transform.translation, 1.f);
        push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
        push.radius = obj.transform.scale.x;

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PointLightPushConstants),
            &push
        );
        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }
};

}
