#include "simple_render_system.hpp"

#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Glorp {

struct SimplePushConstantData {
    glm::mat4 transform{1.f};
    glm::mat4 normalMatrix{1.f};
};

SimpleRenderSystem::SimpleRenderSystem(GlorpDevice &device, VkRenderPass renderPass): m_glorpDevice{device} {
    createPipelineLayout();
    createPipeline(renderPass);
}
SimpleRenderSystem::~SimpleRenderSystem() {
    vkDestroyPipelineLayout(m_glorpDevice.device(), m_pipelineLayout, nullptr);
}


void SimpleRenderSystem::createPipelineLayout() {

    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    if(vkCreatePipelineLayout(m_glorpDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Could not create pipeline layout");
    }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
    assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};

    GlorpPipeline::defaultPipelineConfigInfo(pipelineConfig);

    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_glorpPipeline = std::make_unique<GlorpPipeline>(
        m_glorpDevice,
        std::string(SHADERS_DIR) + "/simple_shader.vert.spv",
        std::string(SHADERS_DIR) + "/simple_shader.frag.spv",
        pipelineConfig
    );
}
void SimpleRenderSystem::renderGameObjects(FrameInfo &frameInfo, std::vector<GlorpGameObject> &gameObjects) {
    m_glorpPipeline->bind(frameInfo.commandBuffer);

    auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

    for(auto &gameObject : gameObjects) {
        SimplePushConstantData push{};
        auto modelMatrix = gameObject.transform.mat4();

        push.transform = projectionView * modelMatrix;
        push.normalMatrix = gameObject.transform.normalMatrix();

        vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        gameObject.model->bind(frameInfo.commandBuffer);
        gameObject.model->draw(frameInfo.commandBuffer);
    }
};
}