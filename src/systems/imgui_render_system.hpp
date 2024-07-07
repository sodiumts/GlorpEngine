#pragma once 

#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_game_object.hpp"
#include "glorp_camera.hpp"
#include "glorp_frame_info.hpp"

#include <memory>
#include <vector>

#ifndef SHADERS_DIR
#define SHADERS_DIR
#endif 

namespace Glorp {
class ImGuiRenderSystem {
    public:
        ImGuiRenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~ImGuiRenderSystem();

        ImGuiRenderSystem(const ImGuiRenderSystem&) = delete;
        ImGuiRenderSystem &operator=(const ImGuiRenderSystem &) = delete;

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