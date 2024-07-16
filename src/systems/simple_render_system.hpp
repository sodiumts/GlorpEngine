#pragma once

#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"

#include <memory>

#ifndef SHADERS_DIR
#define SHADERS_DIR
#endif

namespace Glorp {
class SimpleRenderSystem {
    public:
        SimpleRenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
    private:
        GlorpDevice &m_glorpDevice;

        std::unique_ptr<GlorpPipeline> m_glorpPipeline;
        VkPipelineLayout m_pipelineLayout;
};

}
