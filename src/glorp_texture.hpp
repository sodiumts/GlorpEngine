#pragma once

#include "glorp_device.hpp"
#include <string>
#include "stb_image.h"

namespace Glorp {
class Texture {
    public:
        Texture(GlorpDevice &device, const std::string &filepath);
        ~Texture();

        Texture (const Texture&) = delete;
        Texture& operator= (const Texture&) = delete;

        VkSampler getSampler() { return m_sampler; }

        VkImageView getImageView() { return m_imageView; }
        VkImageLayout getImageLayout() { return m_imageLayout; }
    private:
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void createImage(const std::string &filepath);
        void createSampler();
        void createImageView();
        void generateMipMaps();
    private:

        int m_height, m_width, m_mipLevels;
        stbi_uc* m_imageData;
        GlorpDevice& m_device;
        VkImage m_image;
        VkDeviceMemory m_imageMemory;
        VkImageView m_imageView;
        VkSampler m_sampler;
        VkFormat m_imageFormat;
        VkImageLayout m_imageLayout;
};
}
