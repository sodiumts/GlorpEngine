#include "ps1_render_system.hpp"
#include "glorp_descriptors.hpp"

#include <glm/common.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
namespace Glorp {

struct SimplePushConstantData {
    glm::mat4 modelMatrix{1.f};
    glm::mat4 normalMatrix{1.f};
    glm::vec2 viewportSize{1.f};
};

struct PushFog {
    VkBool32 fogEnabled = VK_TRUE;
    float nearPlane{0.1f};
    float farPlane{1000.f};
    float fogStart{5.f};
    float fogEnd{10.f};
    alignas(16) glm::vec3 fogColor{0.42f, 0.42f, 0.45f};
    alignas(16) glm::vec3 cameraPos;
};

PS1RenderSystem::PS1RenderSystem(GlorpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout): m_glorpDevice{device} {
    scaleUBOBuffer = std::make_unique<GlorpBuffer>(
        device,
        sizeof(ScaleUBO),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    );
    scaleUBOBuffer->map();
    createOffscreenImage();
    createTextureSampler(); 
    createQuadDescriptorSet();
    createPipelineLayout(globalSetLayout, textureSetLayout);
    createPipeline(renderPass);
    createQuadPipeline(renderPass);

}
PS1RenderSystem::~PS1RenderSystem() {
    vkDestroyPipelineLayout(m_glorpDevice.device(), m_pipelineLayout, nullptr);
    vkDestroyImageView(m_glorpDevice.device(), m_colorImageView, nullptr);
    vkDestroyImageView(m_glorpDevice.device(), m_depthImageView, nullptr);
    vkDestroyImage(m_glorpDevice.device(), m_offscreenImage, nullptr);
    vkDestroyImage(m_glorpDevice.device(), m_offscreenDepth, nullptr);
    vkFreeMemory(m_glorpDevice.device(), m_offscreenImageMemory, nullptr);
    vkFreeMemory(m_glorpDevice.device(), m_offscreenDepthMemory, nullptr);
    vkDestroyFramebuffer(m_glorpDevice.device(), m_offscreenFramebuffer, nullptr);
    vkDestroyRenderPass(m_glorpDevice.device(), m_offscreenRenderPass, nullptr);
    vkDestroyPipelineLayout(m_glorpDevice.device(), m_quadPipelineLayout, nullptr);
    vkDestroySampler(m_glorpDevice.device(), m_textureSampler, nullptr);
}


void PS1RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout) {

    VkPushConstantRange pushConstantRange {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SimplePushConstantData);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, textureSetLayout};


    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    if(vkCreatePipelineLayout(m_glorpDevice.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Could not create pipeline layout");
    }
}

void PS1RenderSystem::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    
    if (vkCreateSampler(m_glorpDevice.device(), &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler!");
    }
}

void PS1RenderSystem::createPipeline(VkRenderPass renderPass) {
    assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipelineConfig{};

    GlorpPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineConfig.multisampleInfo.sampleShadingEnable = VK_FALSE;
    pipelineConfig.renderPass = m_offscreenRenderPass;
    pipelineConfig.pipelineLayout = m_pipelineLayout;
    m_glorpPipeline = std::make_unique<GlorpPipeline>(
        m_glorpDevice,
        std::string(RESOURCE_LOCATIONS) + "shaders/ps1_shader.vert.spv",
        std::string(RESOURCE_LOCATIONS) + "shaders/ps1_shader.frag.spv",
        pipelineConfig
    );

}

void PS1RenderSystem::createQuadPipeline(VkRenderPass renderPass) {
    m_quadPipelineLayout = createQuadPipelineLayout();
    PipelineConfigInfo pipelineConfig{};
    GlorpPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = m_quadPipelineLayout;
    
    m_quadPipeline = std::make_unique<GlorpPipeline>(
        m_glorpDevice,
        "shaders/quad.vert.spv",
        "shaders/quad.frag.spv",
        pipelineConfig
    );
}

VkPipelineLayout PS1RenderSystem::createQuadPipelineLayout() {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushFog);



    VkDescriptorSetLayout descriptorSetLayout = m_quadDescriptorSetLayout->getDescriptorSetLayout();
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    
    VkPipelineLayout pipelineLayout;

    
    if (vkCreatePipelineLayout(m_glorpDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create quad pipeline layout!");
    }
    return pipelineLayout;
}

void PS1RenderSystem::createQuadDescriptorSet() {
    m_quadDescriptorPool = GlorpDescriptorPool::Builder(m_glorpDevice)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .build();

    m_quadDescriptorSetLayout = GlorpDescriptorSetLayout::Builder(m_glorpDevice)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_colorImageView;
    imageInfo.sampler = m_textureSampler;

    transitionImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_offscreenDepth, m_glorpDevice.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    ));

    VkDescriptorImageInfo depthImageInfo{};
    depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    depthImageInfo.imageView = m_depthImageView;
    depthImageInfo.sampler = m_textureSampler;


    VkDescriptorBufferInfo bufferInfo = scaleUBOBuffer->descriptorInfo();
    GlorpDescriptorWriter(*m_quadDescriptorSetLayout, *m_quadDescriptorPool)
        .writeBuffer(0, &bufferInfo)
        .writeImage(1, &imageInfo)
        .writeImage(2, &depthImageInfo)
        .build(m_quadDescriptorSet);
}

void PS1RenderSystem::renderGameObjects(FrameInfo &frameInfo) {
    //m_glorpPipeline->bind(frameInfo.commandBuffer);
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_offscreenRenderPass;
    renderPassInfo.framebuffer = m_offscreenFramebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {320, 240};

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(frameInfo.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Set viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 320.0f;
    viewport.height = 240.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(frameInfo.commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {320, 240};
    vkCmdSetScissor(frameInfo.commandBuffer, 0, 1, &scissor);

    m_glorpPipeline->bind(frameInfo.commandBuffer);
    for (auto &kv : frameInfo.gameObjects) {
        auto& obj = kv.second;
        if (obj.model == nullptr) continue;

        std::vector<VkDescriptorSet> descriptors{frameInfo.globalDescriptorSet, obj.descriptorSet};

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0, descriptors.size(),
            descriptors.data(),
            0, nullptr
        );

        SimplePushConstantData push{};
        push.modelMatrix = obj.transform.mat4();
        push.normalMatrix = obj.transform.normalMatrix();
        push.viewportSize = glm::vec2{320.0f, 240.0f};

        vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

        obj.model->bind(frameInfo.commandBuffer);
        obj.model->draw(frameInfo.commandBuffer);
    }
    vkCmdEndRenderPass(frameInfo.commandBuffer);
    
}

void PS1RenderSystem::createOffscreenImage() {
    VkExtent3D extent{320, 240, 1};
    VkImageCreateInfo colorImageInfo = {};
    colorImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    colorImageInfo.imageType = VK_IMAGE_TYPE_2D;
    colorImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorImageInfo.extent = extent;
    colorImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    colorImageInfo.mipLevels = 1;
    colorImageInfo.arrayLayers = 1;
    colorImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    colorImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorImageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    m_glorpDevice.createImageWithInfo(colorImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_offscreenImage, m_offscreenImageMemory);
    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, m_offscreenImage, VK_FORMAT_R8G8B8A8_UNORM);

    VkImageCreateInfo depthImageInfo = {};
    depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
    depthImageInfo.format = m_glorpDevice.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    depthImageInfo.extent = extent;
    depthImageInfo.mipLevels = 1;
    depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthImageInfo.arrayLayers = 1;
    depthImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    m_glorpDevice.createImageWithInfo(depthImageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_offscreenDepth, m_offscreenDepthMemory);
    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, m_offscreenDepth, depthImageInfo.format);

    VkImageViewCreateInfo colorImageCreateInfo{};
    colorImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorImageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorImageCreateInfo.format = colorImageInfo.format;
    colorImageCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    colorImageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorImageCreateInfo.subresourceRange.baseMipLevel = 0;
    colorImageCreateInfo.subresourceRange.baseArrayLayer = 0;
    colorImageCreateInfo.subresourceRange.layerCount = 1;
    colorImageCreateInfo.subresourceRange.levelCount = 1;
    colorImageCreateInfo.image = m_offscreenImage;

    vkCreateImageView(m_glorpDevice.device(), &colorImageCreateInfo, nullptr, &m_colorImageView);

    VkImageViewCreateInfo depthImageCreateInfo{};
    depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthImageCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthImageCreateInfo.format = depthImageInfo.format;
    depthImageCreateInfo.components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY
    };
    depthImageCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthImageCreateInfo.subresourceRange.baseMipLevel = 0;
    depthImageCreateInfo.subresourceRange.baseArrayLayer = 0;
    depthImageCreateInfo.subresourceRange.layerCount = 1;
    depthImageCreateInfo.subresourceRange.levelCount = 1;
    depthImageCreateInfo.image = m_offscreenDepth;

    vkCreateImageView(m_glorpDevice.device(), &depthImageCreateInfo, nullptr, &m_depthImageView);

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_glorpDevice.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    if (vkCreateRenderPass(m_glorpDevice.device(), &renderPassInfo, nullptr, &m_offscreenRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create offscreen render pass");
    }

    std::array<VkImageView, 2> fbAttachments = {m_colorImageView, m_depthImageView};
    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = m_offscreenRenderPass;
    fbInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
    fbInfo.pAttachments = fbAttachments.data();
    fbInfo.width = 320;
    fbInfo.height = 240;
    fbInfo.layers = 1;

    if (vkCreateFramebuffer(m_glorpDevice.device(), &fbInfo, nullptr, &m_offscreenFramebuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create offscreen framebuffer");
    }
}

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || 
           format == VK_FORMAT_D24_UNORM_S8_UINT ||
           format == VK_FORMAT_D16_UNORM_S8_UINT;
}

void PS1RenderSystem::renderToSwapchain(FrameInfo &frameInfo) {
    m_quadPipeline->bind(frameInfo.commandBuffer);
    
    float scaleX = frameInfo.screenx / 320.0f;
    float scaleY = frameInfo.screeny / 240.0f;
    float scaleFactor = floor(glm::max(scaleX, scaleY));

    ScaleUBO ubo{};
    ubo.windowSize = glm::vec2(frameInfo.screenx, frameInfo.screeny);
    ubo.scaleFactor = scaleFactor;
    scaleUBOBuffer->writeToBuffer(&ubo);
    scaleUBOBuffer->flush();

    

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_quadPipelineLayout,
        0, 1, &m_quadDescriptorSet,
        0, nullptr
    );
    
    PushFog push{};
    push.cameraPos = frameInfo.camera.getPosition();
    push.fogEnabled = frameInfo.fogInfo.fogEnabled;
    push.fogColor = frameInfo.fogInfo.fogColor;
    push.fogStart = frameInfo.fogInfo.fogStart;
    push.fogEnd = frameInfo.fogInfo.fogEnd;

    vkCmdPushConstants(frameInfo.commandBuffer, m_quadPipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushFog), &push);
    // Draw fullscreen triangle
    vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
}

void PS1RenderSystem::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkImage &offscreenImage, VkFormat format) {
    VkCommandBuffer commandBuffer = m_glorpDevice.beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = offscreenImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    if (format == VK_FORMAT_D32_SFLOAT ||
        format == VK_FORMAT_D16_UNORM  ||
        format == VK_FORMAT_D16_UNORM_S8_UINT  ||
        format == VK_FORMAT_D24_UNORM_S8_UINT  ||
        format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (hasStencilComponent(format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // Color attachment transition
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    // Depth attachment transition
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && 
            newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        std::cout << "old: " << oldLayout << "new: " << newLayout << std::endl;
        throw std::runtime_error("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    m_glorpDevice.endSingleTimeCommands(commandBuffer);
}


}
