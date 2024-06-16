#pragma once 

#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_game_object.hpp"

#include <memory>
#include <vector>

#ifndef SHADERS_DIR
#define SHADERS_DIR
#endif 

namespace Glorp {
class SimpleRenderSystem {
    public:
        SimpleRenderSystem(GlorpDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GlorpGameObject> &gameObjects);
    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);
    private:
        GlorpDevice &m_glorpDevice;

        std::unique_ptr<GlorpPipeline> m_glorpPipeline;
        VkPipelineLayout m_pipelineLayout;
};

}