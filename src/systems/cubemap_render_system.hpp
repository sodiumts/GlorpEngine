#pragma once

#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"

#include <memory>

#ifndef RESOURCE_LOCATIONS
#define RESOURCE_LOCATIONS ""
#endif

namespace Glorp {
class CubeMapRenderSystem {
    public:
        CubeMapRenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~CubeMapRenderSystem();

        CubeMapRenderSystem(const CubeMapRenderSystem&) = delete;
        CubeMapRenderSystem &operator=(const CubeMapRenderSystem &) = delete;

        void renderCubemap(FrameInfo &frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);
        void createSkyboxCube();
    private:
        GlorpDevice &m_glorpDevice;

        std::unique_ptr<GlorpPipeline> m_glorpPipeline;
        VkPipelineLayout m_pipelineLayout;
        std::unique_ptr<GlorpModel> m_skyboxCube;
};

}
