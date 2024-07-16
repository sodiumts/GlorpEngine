#include "glorp_renderer.hpp"

#include <stdexcept>
#include <array>

namespace Glorp {

GlorpRenderer::GlorpRenderer(GlorpWindow &window, GlorpDevice &device) : m_glorpWindow{window}, m_glorpDevice{device} {
    recreateSwapChain();
    createCommandBuffers();
}
GlorpRenderer::~GlorpRenderer() { freeCommandBuffers(); }

void GlorpRenderer::recreateSwapChain() {
    auto extent = m_glorpWindow.getExtent();
    while(extent.width == 0 || extent.height == 0) {
        extent = m_glorpWindow.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_glorpDevice.device());

    if(m_glorpSwapChain == nullptr) {
        m_glorpSwapChain = std::make_unique<GlorpSwapChain>(m_glorpDevice, extent);
    } else {
        std::shared_ptr<GlorpSwapChain> oldSwapChain = std::move(m_glorpSwapChain);
        m_glorpSwapChain = std::make_unique<GlorpSwapChain>(m_glorpDevice, extent, oldSwapChain);

        if (!oldSwapChain->compareSwapFormats(*m_glorpSwapChain.get())) {
            throw std::runtime_error("Swap chain image format has changed");
        }
    }
}

void GlorpRenderer::createCommandBuffers() {
    m_commandBuffers.resize(GlorpSwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_glorpDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_glorpDevice.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Could not create allocate command buffers");
    }
}
void GlorpRenderer::freeCommandBuffers() {
    vkFreeCommandBuffers(m_glorpDevice.device(), m_glorpDevice.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    m_commandBuffers.clear();
}

VkCommandBuffer GlorpRenderer::beginFrame() {
    assert(!m_isFrameStarted && "Cant begin a frame if frame is already in progress");

    auto result = m_glorpSwapChain->acquireNextImage(&m_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire next swapchain image");
    }

    m_isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Could not begin recording command buffer");
    }

    return commandBuffer;
}
void GlorpRenderer::endFrame() {
    assert(m_isFrameStarted && "Cant call endFrame if frame is not in progress");

    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }
    auto result = m_glorpSwapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_glorpWindow.wasWindowResized()) {
        m_glorpWindow.resetWindowResizedFlag();
        recreateSwapChain();
    } else if(result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }

    m_isFrameStarted = false;
    m_currentFrameIndex = (m_currentFrameIndex + 1) % GlorpSwapChain::MAX_FRAMES_IN_FLIGHT;
}
void GlorpRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(m_isFrameStarted && "Cannot begin swap chain render pass if frame is not started");
    assert(commandBuffer == getCurrentCommandBuffer() && "Cant begin render pass on a command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_glorpSwapChain->getRenderPass();
    renderPassInfo.framebuffer = m_glorpSwapChain->getFrameBuffer(m_currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_glorpSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.height = static_cast<float>(m_glorpSwapChain->getSwapChainExtent().height);
    viewport.width = static_cast<float>(m_glorpSwapChain->getSwapChainExtent().width);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0,0}, m_glorpSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);



}
void GlorpRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
    assert(m_isFrameStarted && "Cannot end swap chain render pass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Cant end render pass on a command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);

}
}
