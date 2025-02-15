#pragma once

#include "glorp_window.hpp"
#include "glorp_device.hpp"
#include "glorp_swap_chain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace Glorp {
class GlorpRenderer {
    public:
        GlorpRenderer(GlorpWindow &window, GlorpDevice &device);
        ~GlorpRenderer();

        GlorpRenderer(const GlorpRenderer&) = delete;
        GlorpRenderer &operator=(const GlorpRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return m_glorpSwapChain->getRenderPass(); }
        float getAspectRatio() const { return m_glorpSwapChain->extentAspectRatio(); }
        bool isFrameInProgress() const { return m_isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(m_isFrameStarted && "Cannot get current command buffer while the frame is not in progress");
            return m_commandBuffers[m_currentFrameIndex];
        }
        int getFrameIndex() const {
            assert(m_isFrameStarted && "Cannot get current frame index while the frame is not in progress");
            return m_currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void recreateSwapChain();


    private:
        void createCommandBuffers();
        void freeCommandBuffers();
    private:
        GlorpWindow &m_glorpWindow;
        GlorpDevice &m_glorpDevice;
        std::unique_ptr<GlorpSwapChain> m_glorpSwapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;

        uint32_t m_currentImageIndex;
        int m_currentFrameIndex = 0;
        bool m_isFrameStarted = false;
};
}
