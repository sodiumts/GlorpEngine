#include "glorp_cubemap.hpp"
#include "stb_image.h"
#include "glorp_buffer.hpp"

namespace Glorp {

GlorpCubeMap::GlorpCubeMap(GlorpDevice &device, const std::vector<std::string> &filenames) : m_device(device) {
    createCubemapImage(filenames);
    createImageView();
    createSampler();
}

GlorpCubeMap::~GlorpCubeMap(){
    vkDestroyImage(m_device.device(), m_image, nullptr);
    vkFreeMemory(m_device.device(), m_imageMemory, nullptr);
    vkDestroyImageView(m_device.device(), m_imageView, nullptr);
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}


void GlorpCubeMap::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = m_device.beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 6;
    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::runtime_error("Unsupported layout transition");
    }
    vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    m_device.endSingleTimeCommands(commandBuffer);
    m_imageLayout = newLayout;
}


void GlorpCubeMap::createSampler() {
    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    if(vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler.");
    }
}

void GlorpCubeMap::createImageView() {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.image = m_image;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1, 0, 6
    };
    if(vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view.");
    }

}

void GlorpCubeMap::createCubemapImage(const std::vector<std::string> &filenames) {
    // Order should be right, left, top, bottom, front, back
    std::vector<stbi_uc*> pixels;
    for(const auto& path: filenames) {
        stbi_set_flip_vertically_on_load(true);
        stbi_uc* image = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
        pixels.push_back(image);
    }    
    
    VkDeviceSize faceSize = m_width * m_height * 4;
    VkDeviceSize bufferSize = faceSize * 6;

    GlorpBuffer stagingBuffer {
        m_device,
        1, 
        static_cast<uint32_t>(bufferSize),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    
    stagingBuffer.map();
    for(size_t i = 0; i < 6; i++) {
        void* offset = static_cast<char*>(stagingBuffer.getMappedMemory()) + (i * faceSize);
        memcpy(offset, pixels[i], faceSize);
    }
    stagingBuffer.unmap();

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.extent = {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 6;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);
    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    VkCommandBuffer commandBuffer = m_device.beginSingleTimeCommands();

    std::vector<VkBufferImageCopy> regions;
    for(uint32_t i = 0; i < 6; i++) {
        VkBufferImageCopy region{};
        region.bufferOffset = i * faceSize;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = i;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = {static_cast<uint32_t>(m_width), 
            static_cast<uint32_t>(m_height), 1};
        regions.push_back(region);
    }

    vkCmdCopyBufferToImage(
            commandBuffer,
            stagingBuffer.getBuffer(),
            m_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(regions.size()),
            regions.data()
            );

    m_device.endSingleTimeCommands(commandBuffer);

    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    for(auto& pixel : pixels) {
        stbi_image_free(pixel);
    }
}

}
