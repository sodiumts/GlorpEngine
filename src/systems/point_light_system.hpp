#pragma once

#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"

#include <memory>

#ifndef RESOURCE_LOCATIONS
#define RESOURCE_LOCATIONS ""
#endif

namespace Glorp {
class PointLightSystem {
    public:
        PointLightSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void update(FrameInfo &frameInfo, GlobalUbo &ubo);
        void render(FrameInfo &frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
    private:
        GlorpDevice &m_glorpDevice;

        std::unique_ptr<GlorpPipeline> m_glorpPipeline;
        VkPipelineLayout m_pipelineLayout;
};

}
