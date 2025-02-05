#pragma once

#include "glorp_device.hpp"
#include "stb_image.h"

namespace Glorp {
class GlorpCubeMap {
    public:
        GlorpCubeMap(GlorpDevice &device, const std::vector<std::string> &filenames);
        ~GlorpCubeMap();

        GlorpCubeMap (const GlorpCubeMap&) = delete;
        GlorpCubeMap& operator= (const GlorpCubeMap&) = delete;

        VkSampler getSampler() { return m_sampler; }

        VkImageView getImageView() { return m_imageView; }
        VkImageLayout getImageLayout() { return m_imageLayout; }
    private:
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void createSampler();
        void createImageView();

        void createCubemapImage(const std::vector<std::string> &filenames);
    private:

        int m_height, m_width, m_channels;
        GlorpDevice& m_device;
        VkImage m_image;
        VkDeviceMemory m_imageMemory;
        VkImageView m_imageView;
        VkSampler m_sampler;
        VkFormat m_imageFormat;
        VkImageLayout m_imageLayout;
};
}
