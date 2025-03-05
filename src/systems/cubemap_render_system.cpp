#include "cubemap_render_system.hpp"
#include "glorp_game_object.hpp"

namespace Glorp {

CubeMapRenderSystem::CubeMapRenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout): m_glorpDevice(device) {
    createSkyboxCube();
    createPipelineLayout(globalSetLayout);
    createPipeline(renderPass);
}

CubeMapRenderSystem::~CubeMapRenderSystem() {
    vkDestroyPipelineLayout(m_glorpDevice.device(), m_pipelineLayout, nullptr);
}


void CubeMapRenderSystem::renderCubemap(FrameInfo &frameInfo) {
    m_glorpPipeline->bind(frameInfo.commandBuffer);
    
    std::vector<VkDescriptorSet> descriptors = {frameInfo.globalDescriptorSet};
    
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, static_cast<uint32_t>(descriptors.size()), descriptors.data(), 0, nullptr);
    m_skyboxCube->bind(frameInfo.commandBuffer);
    m_skyboxCube->draw(frameInfo.commandBuffer);
}

void CubeMapRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_glorpDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void CubeMapRenderSystem::createPipeline(VkRenderPass renderPass) {
    
    PipelineConfigInfo pipelineConfig{};
    GlorpPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.multisampleInfo.rasterizationSamples = m_glorpDevice.getSupportedSampleCount();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
    pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineConfig.bindingDescriptions = {GlorpModel::Vertex::getBindingDescriptions()[0]};
    pipelineConfig.attributeDescriptions = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(GlorpModel::Vertex, position)}
    };

    m_glorpPipeline = std::make_unique<GlorpPipeline>(
            m_glorpDevice,
            std::string(RESOURCE_LOCATIONS) + "shaders/cubemap.vert.spv",
            std::string(RESOURCE_LOCATIONS) + "shaders/cubemap.frag.spv",
            pipelineConfig
    );
}
void CubeMapRenderSystem::createSkyboxCube() {
    std::vector<GlorpModel::Vertex> vertices = {
        // Front face
        {{-1.0f, -1.0f,  1.0f}, {1,1,1}, {0,0,1}, {0,1}, {}, {}},
        {{ 1.0f, -1.0f,  1.0f}, {1,1,1}, {0,0,1}, {1,1}, {}, {}},
        {{ 1.0f,  1.0f,  1.0f}, {1,1,1}, {0,0,1}, {1,0}, {}, {}},
        {{-1.0f,  1.0f,  1.0f}, {1,1,1}, {0,0,1}, {0,0}, {}, {}},

        // Back face
        {{-1.0f, -1.0f, -1.0f}, {1,1,1}, {0,0,-1}, {1,1}, {}, {}},
        {{ 1.0f, -1.0f, -1.0f}, {1,1,1}, {0,0,-1}, {0,1}, {}, {}},
        {{ 1.0f,  1.0f, -1.0f}, {1,1,1}, {0,0,-1}, {0,0}, {}, {}},
        {{-1.0f,  1.0f, -1.0f}, {1,1,1}, {0,0,-1}, {1,0}, {}, {}},

        // Left face
        {{-1.0f, -1.0f, -1.0f}, {1,1,1}, {-1,0,0}, {0,1}, {}, {}},
        {{-1.0f, -1.0f,  1.0f}, {1,1,1}, {-1,0,0}, {1,1}, {}, {}},
        {{-1.0f,  1.0f,  1.0f}, {1,1,1}, {-1,0,0}, {1,0}, {}, {}},
        {{-1.0f,  1.0f, -1.0f}, {1,1,1}, {-1,0,0}, {0,0}, {}, {}},

        // Right face
        {{ 1.0f, -1.0f, -1.0f}, {1,1,1}, {1,0,0}, {1,1}, {}, {}},
        {{ 1.0f, -1.0f,  1.0f}, {1,1,1}, {1,0,0}, {0,1}, {}, {}},
        {{ 1.0f,  1.0f,  1.0f}, {1,1,1}, {1,0,0}, {0,0}, {}, {}},
        {{ 1.0f,  1.0f, -1.0f}, {1,1,1}, {1,0,0}, {1,0}, {}, {}},

        // Top face
        {{-1.0f,  1.0f, -1.0f}, {1,1,1}, {0,1,0}, {0,1}, {}, {}},
        {{ 1.0f,  1.0f, -1.0f}, {1,1,1}, {0,1,0}, {1,1}, {}, {}},
        {{ 1.0f,  1.0f,  1.0f}, {1,1,1}, {0,1,0}, {1,0}, {}, {}},
        {{-1.0f,  1.0f,  1.0f}, {1,1,1}, {0,1,0}, {0,0}, {}, {}},

        // Bottom face
        {{-1.0f, -1.0f, -1.0f}, {1,1,1}, {0,-1,0}, {1,1}, {}, {}},
        {{ 1.0f, -1.0f, -1.0f}, {1,1,1}, {0,-1,0}, {0,1}, {}, {}},
        {{ 1.0f, -1.0f,  1.0f}, {1,1,1}, {0,-1,0}, {0,0}, {}, {}},
        {{-1.0f, -1.0f,  1.0f}, {1,1,1}, {0,-1,0}, {1,0}, {}, {}},
    };

    
    GlorpModel::Builder builder;
    builder.indices = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 5, 6, 6, 7, 4,
        // Left face
        8, 9, 10, 10, 11, 8,
        // Right face
        12, 13, 14, 14, 15, 12,
        // Top face
        16, 17, 18, 18, 19, 16,
        // Bottom face
        20, 21, 22, 22, 23, 20
    };

    builder.vertices = vertices;
    m_skyboxCube = std::make_unique<GlorpModel>(m_glorpDevice, builder);
}
}
