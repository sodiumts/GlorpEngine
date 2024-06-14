#pragma once 

#include "glorp_window.hpp"
#include "glorp_pipeline.hpp"
#include "glorp_device.hpp"
#include "glorp_swap_chain.hpp"
#include "glorp_model.hpp"

#include <memory>
#include <vector>

#ifndef SHADERS_DIR
#define SHADERS_DIR
#endif 

namespace Glorp {
class FirstApp {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp&) = delete;
        FirstApp &operator=(const FirstApp &) = delete;
        
        void run();
    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);
    private:
        GlorpWindow m_glorpWindow {WIDTH, HEIGHT, "First App!"};
        GlorpDevice m_glorpDevice {m_glorpWindow};
        std::unique_ptr<GlorpSwapChain> m_glorpSwapChain;

        std::unique_ptr<GlorpPipeline> m_glorpPipeline;
        VkPipelineLayout m_pipelineLayout;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::unique_ptr<GlorpModel> m_glorpModel;
};

}