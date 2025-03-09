#pragma once

#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"

#include <memory>
#include <vulkan/vulkan_core.h>

#ifndef RESOURCE_LOCATIONS
#define RESOURCE_LOCATIONS ""
#endif

namespace Glorp {
class CloudRenderSystem {
    public:
        CloudRenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout skySetLayout);
        ~CloudRenderSystem();

        CloudRenderSystem(const CloudRenderSystem&) = delete;
        CloudRenderSystem &operator=(const CloudRenderSystem &) = delete;

        void renderClouds(FrameInfo &frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout skySetLayout);
        void createPipeline(VkRenderPass renderPass);
        void createFullscreenQuad();
    private:
        GlorpDevice &m_glorpDevice;

        std::unique_ptr<GlorpPipeline> m_glorpPipeline;
        VkPipelineLayout m_pipelineLayout;
        std::unique_ptr<GlorpModel> m_fullQuad; 
};

}
