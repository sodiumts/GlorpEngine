#pragma once

#include "glorp_descriptors.hpp"
#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_frame_info.hpp"

#include <memory>
#include <vulkan/vulkan_core.h>

#ifndef RESOURCE_LOCATIONS
#define RESOURCE_LOCATIONS ""
#endif


namespace Glorp {

struct ScaleUBO {
    glm::vec2 windowSize;
    float scaleFactor;
};


class PS1RenderSystem {
    public:
        PS1RenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
        ~PS1RenderSystem();

        PS1RenderSystem(const PS1RenderSystem&) = delete;
        PS1RenderSystem &operator=(const PS1RenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo);
        void renderToSwapchain(FrameInfo &frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
        void createPipeline(VkRenderPass renderPass);
        void createOffscreenImage();
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkImage &offscreenImage, VkFormat format);
        void createQuadPipeline(VkRenderPass renderPass);
        void createQuadDescriptorSet();
        VkPipelineLayout createQuadPipelineLayout();
        void createTextureSampler();
    private:
        GlorpDevice &m_glorpDevice;

        std::unique_ptr<GlorpPipeline> m_glorpPipeline;
        std::unique_ptr<GlorpPipeline> m_quadPipeline;
        VkDescriptorSet m_quadDescriptorSet;
        VkSampler m_textureSampler;

        VkPipelineLayout m_pipelineLayout;
        VkImage m_offscreenImage;
        VkImageView m_colorImageView;
        VkDeviceMemory m_offscreenImageMemory;

        VkImage m_offscreenDepth;
        VkImageView m_depthImageView;
        VkDeviceMemory m_offscreenDepthMemory;

        VkRenderPass m_offscreenRenderPass;
        VkFramebuffer m_offscreenFramebuffer;

        VkDescriptorSet m_mainDescriptorSet;
        
        std::unique_ptr<GlorpDescriptorPool> m_quadDescriptorPool;
        std::unique_ptr<GlorpDescriptorSetLayout> m_quadDescriptorSetLayout;

        VkPipelineLayout m_quadPipelineLayout;

        std::unique_ptr<GlorpBuffer> scaleUBOBuffer;
};

}
