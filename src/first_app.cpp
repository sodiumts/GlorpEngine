#include "first_app.hpp"

#include <stdexcept>
#include <array>

namespace Glorp {
FirstApp::FirstApp() {
    loadModels();
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}
FirstApp::~FirstApp() {
    vkDestroyPipelineLayout(m_glorpDevice.device(), m_pipelineLayout, nullptr);
}


void FirstApp::run() {
    while(!m_glorpWindow.shouldClose()) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(m_glorpDevice.device());
}


void FirstApp::loadModels() {
    std::vector<GlorpModel::Vertex> vertices {
        {{0.0f, -0.5f}},
        {{0.5f, 0.5f}},
        {{-0.5f, 0.5f}}
    };

    m_glorpModel = std::make_unique<GlorpModel>(m_glorpDevice, vertices);
}

 void FirstApp::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    if(vkCreatePipelineLayout(m_glorpDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Could not create pipeline layout");
    }
 }

void FirstApp::createPipeline() {
    auto pipelineConfig = GlorpPipeline::defaultPipelineConfigInfo(m_glorpSwapChain.width(), m_glorpSwapChain.height());
    pipelineConfig.renderPass = m_glorpSwapChain.getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_glorpPipeline = std::make_unique<GlorpPipeline>(
        m_glorpDevice,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        pipelineConfig
    );
}

void FirstApp::createCommandBuffers() {
    m_commandBuffers.resize(m_glorpSwapChain.imageCount());

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_glorpDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_glorpDevice.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Could not create allocate command buffers");
    }

    for (int i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Could not begin recording command buffer");
        }
        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_glorpSwapChain.getRenderPass();
        renderPassInfo.framebuffer = m_glorpSwapChain.getFrameBuffer(i);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_glorpSwapChain.getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_glorpPipeline->bind(m_commandBuffers[i]);
        m_glorpModel->bind(m_commandBuffers[i]);
        m_glorpModel->draw(m_commandBuffers[i]);

        vkCmdEndRenderPass(m_commandBuffers[i]);
        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }
    }   

}
void FirstApp::drawFrame() {
    uint32_t imageIndex;
    auto result = m_glorpSwapChain.acquireNextImage(&imageIndex);

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire next swapchain image");
    }

    result = m_glorpSwapChain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }
};

}