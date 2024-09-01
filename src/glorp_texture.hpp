#pragma once

#include "glorp_device.hpp"


#include "tiny_gltf.h"

namespace Glorp {
class GlorpTexture {
    public:
        GlorpTexture(GlorpDevice &device, const tinygltf::Image &image);
        ~GlorpTexture();

        GlorpTexture (const GlorpTexture&) = delete;
        GlorpTexture& operator= (const GlorpTexture&) = delete;

        VkSampler getSampler() { return m_sampler; }

        VkImageView getImageView() { return m_imageView; }
        VkImageLayout getImageLayout() { return m_imageLayout; }
    private:
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void createSampler();
        void createImageView();
        void generateMipMaps();

        void createImageGLTF(const tinygltf::Image &image);
    private:

        int m_height, m_width, m_mipLevels;
        GlorpDevice& m_device;
        VkImage m_image;
        VkDeviceMemory m_imageMemory;
        VkImageView m_imageView;
        VkSampler m_sampler;
        VkFormat m_imageFormat;
        VkImageLayout m_imageLayout;
};
}
