#pragma once

#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"

#include <memory>

#ifndef RESOURCE_LOCATIONS
#define RESOURCE_LOCATIONS ""
#endif

namespace Glorp {
class SimpleRenderSystem {
    public:
        SimpleRenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
        void createPipeline(VkRenderPass renderPass);
    private:
        GlorpDevice &m_glorpDevice;

        std::unique_ptr<GlorpPipeline> m_glorpPipeline;
        VkPipelineLayout m_pipelineLayout;
        std::unique_ptr<GlorpPipeline> m_quadPipeline;
        
};

}
