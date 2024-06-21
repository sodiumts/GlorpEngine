#pragma once

#include "glorp_device.hpp"

#include <string>
#include <vector>

namespace Glorp {

struct PipelineConfigInfo {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;


    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class GlorpPipeline {
    public:
        GlorpPipeline(GlorpDevice& device, const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);
        ~GlorpPipeline();

        GlorpPipeline(const GlorpPipeline&) = delete;
        GlorpPipeline &operator=(const GlorpPipeline&) = delete;

        static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);
        void bind(VkCommandBuffer commandBuffer);

    private:
        static std::vector<char> readFile(const std::string& filePath);

        void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo configInfo);

        void createShaderModule(const std::vector<char>& shaderCode, VkShaderModule* shaderModule);
    private:
        GlorpDevice &m_glorpDevice;
        VkPipeline m_graphicsPipeline;
        VkShaderModule m_vertShaderModule;
        VkShaderModule m_fragShaderModule;
};
}