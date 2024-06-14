#include "first_app.hpp"

#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Glorp {

struct SimplePushConstantData {
    glm::vec2 offset; 
    alignas(16) glm::vec3 color;
};

FirstApp::FirstApp() {
    loadModels();
    createPipelineLayout();
    recreateSwapChain();
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
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    m_glorpModel = std::make_unique<GlorpModel>(m_glorpDevice, vertices);
}

 void FirstApp::createPipelineLayout() {

    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    if(vkCreatePipelineLayout(m_glorpDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Could not create pipeline layout");
    }
 }

void FirstApp::createPipeline() {
    assert(m_glorpSwapChain != nullptr && "Cannot create pipeline before swap chain");
    assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};

    GlorpPipeline::defaultPipelineConfigInfo(pipelineConfig);

    pipelineConfig.renderPass = m_glorpSwapChain->getRenderPass();
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_glorpPipeline = std::make_unique<GlorpPipeline>(
        m_glorpDevice,
        std::string(SHADERS_DIR) + "/simple_shader.vert.spv",
        std::string(SHADERS_DIR) + "/simple_shader.frag.spv",
        pipelineConfig
    );
}
void FirstApp::recreateSwapChain() {
    auto extent = m_glorpWindow.getExtent();
    while(extent.width == 0 || extent.height == 0) {
        extent = m_glorpWindow.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_glorpDevice.device());

    if(m_glorpSwapChain == nullptr) {
        m_glorpSwapChain = std::make_unique<GlorpSwapChain>(m_glorpDevice, extent);
    } else {
        m_glorpSwapChain = std::make_unique<GlorpSwapChain>(m_glorpDevice, extent, std::move(m_glorpSwapChain));
        if(m_glorpSwapChain->imageCount() != m_commandBuffers.size()) {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }
    createPipeline();
}

void FirstApp::createCommandBuffers() {
    m_commandBuffers.resize(m_glorpSwapChain->imageCount());

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_glorpDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_glorpDevice.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Could not create allocate command buffers");
    }
}
void FirstApp::freeCommandBuffers() {
    vkFreeCommandBuffers(m_glorpDevice.device(), m_glorpDevice.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
    m_commandBuffers.clear();
}


void FirstApp::recordCommandBuffer(int imageIndex) {
    static int frame = 0;
    
    frame = (frame + 1) % 1000;

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Could not begin recording command buffer");
    }
    VkRenderPassBeginInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_glorpSwapChain->getRenderPass();
    renderPassInfo.framebuffer = m_glorpSwapChain->getFrameBuffer(imageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_glorpSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.height = static_cast<float>(m_glorpSwapChain->getSwapChainExtent().height);
    viewport.width = static_cast<float>(m_glorpSwapChain->getSwapChainExtent().width);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0,0}, m_glorpSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
    vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);


    m_glorpPipeline->bind(m_commandBuffers[imageIndex]);
    m_glorpModel->bind(m_commandBuffers[imageIndex]);

    for (int i = 0; i < 4; i++) {
        SimplePushConstantData push{};
        push.offset = {-0.5f + frame * 0.002f, -0.4f + i * 0.25f};
        push.color = {0.0f, 0.0f, 0.2f + 0.2f*i};
        vkCmdPushConstants(m_commandBuffers[imageIndex], m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
        m_glorpModel->draw(m_commandBuffers[imageIndex]);
    }



    vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
    if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }
}
void FirstApp::drawFrame() {
    uint32_t imageIndex;
    auto result = m_glorpSwapChain->acquireNextImage(&imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire next swapchain image");
    }
    recordCommandBuffer(imageIndex);
    result = m_glorpSwapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_glorpWindow.wasWindowResized()) {
        m_glorpWindow.resetWindowResizedFlag();
        recreateSwapChain();
        return;
    }

    if(result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }
};

}