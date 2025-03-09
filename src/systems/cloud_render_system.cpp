#include "cloud_render_system.hpp"
#include <vulkan/vulkan_core.h>

namespace Glorp {

struct CloudPushConstantData {
    glm::vec3 sunDirection;
};
CloudRenderSystem::CloudRenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout skySetLayout): m_glorpDevice(device) {
    createFullscreenQuad();
    createPipelineLayout(globalSetLayout, skySetLayout);
    createPipeline(renderPass);
}

CloudRenderSystem::~CloudRenderSystem() {
    vkDestroyPipelineLayout(m_glorpDevice.device(), m_pipelineLayout, nullptr);
}


void CloudRenderSystem::renderClouds(FrameInfo &frameInfo) {
    m_glorpPipeline->bind(frameInfo.commandBuffer);
    
    std::vector<VkDescriptorSet> descriptors = {frameInfo.globalDescriptorSet, frameInfo.skyDescriptorSet};
    
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, static_cast<uint32_t>(descriptors.size()), descriptors.data(), 0, nullptr);

    CloudPushConstantData push{};
    push.sunDirection = frameInfo.sunDirection;
    
    vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(CloudPushConstantData), &push);
    
    m_fullQuad->bind(frameInfo.commandBuffer);
    m_fullQuad->draw(frameInfo.commandBuffer);
}

void CloudRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout skySetLayout) {
    
    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(CloudPushConstantData);


    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {globalSetLayout, skySetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_glorpDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void CloudRenderSystem::createPipeline(VkRenderPass renderPass) {
    
    PipelineConfigInfo pipelineConfig{};
    GlorpPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.multisampleInfo.rasterizationSamples = m_glorpDevice.getSupportedSampleCount();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
    pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
    pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.bindingDescriptions = {GlorpModel::Vertex::getBindingDescriptions()[0]};
    pipelineConfig.attributeDescriptions = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GlorpModel::Vertex, position)},
    };

    m_glorpPipeline = std::make_unique<GlorpPipeline>(
            m_glorpDevice,
            std::string(RESOURCE_LOCATIONS) + "shaders/clouds.vert.spv",
            std::string(RESOURCE_LOCATIONS) + "shaders/clouds.frag.spv",
            pipelineConfig
    );
}
void CloudRenderSystem::createFullscreenQuad() {
    GlorpModel::Builder builder;
    std::vector<GlorpModel::Vertex> vertices = {
            {{-1, -1, 0}, {0, 0, 0}},
            {{1, -1, 0}, {1, 0, 0}},
            {{1, 1, 0}, {1, 1, 0}},
            {{-1, 1, 0}, {0, 1, 0}}
        };
    builder.indices = { 0, 1, 2, 2, 3, 0 };

    builder.vertices = vertices;
    m_fullQuad = std::make_unique<GlorpModel>(m_glorpDevice, builder);
}
}
